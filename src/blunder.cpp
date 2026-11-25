#include <blunder/blunder.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <exception>
#include <iostream>
#include <mutex>
#include <optional>
#include <queue>
#include <random>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <blunder/scene.hpp>
#include <geom3/line3.hpp>
#include <geom3/base.hpp>
#include <blunder/utils/scoped_log_timer.hpp>

namespace
{
    // constexpr int SamplesPerPixel = 20;
    // constexpr double RayDirectionStandardDeviation = 0.001;
    // constexpr int DepthLimit = 2;

    constexpr int SamplesPerPixel = 1; // 50
    constexpr double RayDirectionStandardDeviation = 0; // 0.001
    constexpr int DepthLimit = 0; // 10

    double shadow_attenuation(const Scene &scene, const Line3 &ray) {
        // TODO: This is a very naive shadow attenuation implementation. It should be improved to support multiple light sources and partial occlusion.
        for (const std::unique_ptr<Object> &object : scene.objects) {
            const std::optional<Hit> hit = object->closest_intersection(ray);
            if (hit && hit->lambda > 0.0 && hit->lambda < 1.0 && ray.a.distance(ray.calc_point(hit->lambda)) > 0.001){
                return 1.0;
            }
        }
        return 0.0;
    }

    std::uint8_t to_channel(double value) {
        const double clamped = std::clamp(value, 0.0, 1.0);
        return static_cast<std::uint8_t>(clamped * 255.0);
    }
}

Blunder::Blunder()
    : scene_(std::make_unique<Scene>()){
}

Blunder::~Blunder() = default;

Color Blunder::light_contribution(
    const Color &surface_color,
    const Light &light,
    const Vector3 &hit_point,
    const Vector3 &normal) const
{
    const Vector3 direction_to_light = light.direction_to_light(hit_point);
    if (direction_to_light.dot(normal) <= 0.0) {
        return Color{};
    }

    const double intensity = light.intensity(hit_point, normal);

    double shadow_factor = 0.0;
    if (light.can_be_eclipsed()) {
        const Line3 shadow_ray{hit_point, hit_point + direction_to_light};
        shadow_factor = shadow_attenuation(*scene_, shadow_ray);
    }

    Color contribution = surface_color * light.color() * (intensity * (1.0 - shadow_factor));
    if (light.is_specular()) {
        const Vector3 incoming = -1.0 * direction_to_light;
        const Vector3 reflected = incoming.reflect(normal).normalized();
        const Vector3 view = (scene_->camera.position - hit_point).normalized();
        const double specular = std::pow(reflected.dot(view), 7.0);
        if (specular > 0.0){
            contribution = contribution + light.color() * (specular * (1.0 - shadow_factor));
        }
    }

    return contribution;
}

void Blunder::RenderImage()
{
    {
        const ScopedLogTimer timer("transform_objects");
        scene_->transform_objects();
    }

    {
        const ScopedLogTimer timer("Pixel rendering");
        std::queue<int> line_queue;
        for (int y = 0; y < PixelHeight; ++y) {
            line_queue.push(y);
        }

        std::mutex queue_mutex;
        int completed_lines = 0;
        std::exception_ptr render_exception;
        const unsigned int reported_worker_count = std::thread::hardware_concurrency();
        const std::size_t worker_count = std::min<std::size_t>(
            PixelHeight,
            reported_worker_count == 0 ? 1 : reported_worker_count);
        const auto render_lines = [this, &line_queue, &queue_mutex, &completed_lines, &render_exception]() {
            while (true) {
                int y = 0;
                {
                    std::lock_guard<std::mutex> lock(queue_mutex);
                    if (render_exception || line_queue.empty()) {
                        return;
                    }
                    y = line_queue.front();
                    line_queue.pop();
                    
                    std::cout << "\rRendering line " << completed_lines << "/" << PixelHeight << std::flush;
                    ++completed_lines;
                }

                try {
                    for (int x = 0; x < PixelWidth; ++x) {
                        RenderPixel(x, y);
                    }
                } catch (...) {
                    std::lock_guard<std::mutex> lock(queue_mutex);
                    if (!render_exception) {
                        render_exception = std::current_exception();
                    }
                    while (!line_queue.empty()) {
                        line_queue.pop();
                    }
                    return;
                }
            }
        };

        std::vector<std::thread> workers;
        workers.reserve(worker_count);
        std::exception_ptr worker_creation_exception;
        try {
            for (std::size_t i = 0; i < worker_count; ++i) {
                workers.emplace_back(render_lines);
            }
        } catch (...) {
            worker_creation_exception = std::current_exception();
            std::lock_guard<std::mutex> lock(queue_mutex);
            while (!line_queue.empty()) {
                line_queue.pop();
            }
        }
        for (std::thread& worker : workers) {
            worker.join();
        }
        if (render_exception) {
            std::rethrow_exception(render_exception);
        }
        if (worker_creation_exception) {
            std::rethrow_exception(worker_creation_exception);
        }
        std::cout << std::endl;
    }
}

Color Blunder::get_incomming_light(const Scene &scene, const Line3 &ray)
{
    if (RayDirectionStandardDeviation <= 0.0)
    {
        return trace_ray(scene, ray, 0);
    }

    const Vector3 direction = ray.direction();
    const Vector3 reference_axis =
        std::abs(direction.z) < 0.999 ? Vector3{0.0, 0.0, 1.0} : Vector3{0.0, 1.0, 0.0};
    const Vector3 tangent = direction.cross(reference_axis).normalized();
    const Vector3 bitangent = direction.cross(tangent).normalized();
    std::normal_distribution<double> angular_offset(0.0, RayDirectionStandardDeviation);
    thread_local std::mt19937 random_engine(std::random_device{}());

    Color total_color;
    for (int sample = 0; sample < SamplesPerPixel; ++sample)
    {
        const Vector3 sampled_direction =
            (direction + tangent * angular_offset(random_engine) +
             bitangent * angular_offset(random_engine))
                .normalized();
        const Line3 sampled_ray{ray.a, ray.a + sampled_direction};
        total_color = total_color + trace_ray(scene, sampled_ray, 0);
    }

    return total_color / SamplesPerPixel;
}

Color Blunder::trace_ray(const Scene &scene, const Line3 &ray, int depth)
{
    const std::optional<Hit> hit = scene.closest_hit(ray);
    if (!hit)
    {
        return Color{};
    }

    const Vector3 hit_point = ray.calc_point(hit->lambda);
    const Vector3& normal = hit->normal;
    const Color surface_color = hit->object->albedo_at(hit_point);
    Color color;
    for (const std::unique_ptr<Light> &light : scene.lights)
    {
        color = color + this->light_contribution(surface_color, *light, hit_point, normal);
    }

    // Trivial case of recursion: stop bouncing rays after a certain depth to avoid infinite recursion and excessive computation.
    if (depth >= DepthLimit)
    {
        return color;
    }
    // Reflective contribution
    const Vector3 reflected = ray.direction().reflect(normal).normalized();
    const Line3 reflected_ray{hit_point + reflected * epsilon, hit_point + reflected};

    Color reflective_color = this->trace_ray(scene, reflected_ray, depth + 1);

    double reflectivity = hit->object->material().reflectivity();

    return color * (1.0 - reflectivity) + reflective_color * reflectivity;
}

Vector2 Blunder::screen_coords_to_camera(int x, int y) const
{
    const Vector2 pixel_center{static_cast<double>(x) + 0.5, static_cast<double>(y) + 0.5};
    return {
        pixel_center.x / PixelWidth - 0.5,
        pixel_center.y / PixelHeight - 0.5};
}

std::optional<Hit> Blunder::get_hit_info(int x, int y) const
{
    if (x < 0 || x >= PixelWidth || y < 0 || y >= PixelHeight)
    {
        throw std::out_of_range("Pixel coordinates are outside the render grid");
    }

    const Camera& camera = scene_->camera;
    const Vector2 camera_coords = screen_coords_to_camera(x, PixelHeight - y - 1);
    const Vector3 pixel_world_position = camera.camera_coords_to_world(camera_coords);
    const Line3 ray{camera.position, pixel_world_position};
    return scene_->closest_hit(ray);
}

void Blunder::RenderPixel(int x, int y)
{
    if (x < 0 || x >= PixelWidth || y < 0 || y >= PixelHeight)
    {
        throw std::out_of_range("Pixel coordinates are outside the render grid");
    }

    const Camera &camera = scene_->camera;
    const Vector2 camera_coords = screen_coords_to_camera(x, y);
    const Vector3 pixel_world_position = camera.camera_coords_to_world(camera_coords);
    const Line3 ray{camera.position, pixel_world_position};
    const Color final_color = this->get_incomming_light(*scene_, ray);
    pixels_[PixelHeight - y - 1][x] = SDL_Color{
        to_channel(final_color.r()),
        to_channel(final_color.g()),
        to_channel(final_color.b()),
        255};
}

/// Returns this instance's pixel image.
/// @return The generated RGBA pixel grid.
const Blunder::PixelGrid &Blunder::GetPixelGrid() const
{
    return pixels_;
}