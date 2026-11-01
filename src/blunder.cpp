#include <blunder/blunder.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <optional>
#include <random>
#include <stdexcept>

#include <blunder/scene.hpp>
#include <geom3/line3.hpp>

namespace
{
constexpr int SamplesPerPixel = 100;

struct Hit
{
    double lambda;
    const Raytraceable* object;
};

Vector3 multiply(const Vector3& left, const Vector3& right)
{
    return {left.x * right.x, left.y * right.y, left.z * right.z};
}

std::optional<Hit> closest_hit(const Scene& scene, const Line3& ray)
{
    std::optional<Hit> closest;
    double closest_lambda = std::numeric_limits<double>::max();
    for (const Raytraceable& object : scene.objects) {
        const std::optional<double> intersection = object.closest_intersection(ray);
        if (intersection && *intersection > 0.0 && *intersection < closest_lambda) {
            closest_lambda = *intersection;
            closest = Hit{*intersection, &object};
        }
    }
    return closest;
}

double shadow_attenuation(const Scene& scene, const Line3& ray)
{
    for (const Raytraceable& object : scene.objects) {
        const std::optional<double> intersection = object.closest_intersection(ray);
        if (intersection && *intersection > 0.001 && *intersection < 1.0) {
            return 1.0;
        }
    }
    return 0.0;
}

std::uint8_t to_channel(double value)
{
    const double clamped = std::clamp(value, 0.0, 1.0);
    return static_cast<std::uint8_t>(clamped * 255.0);
}
}

Blunder::Blunder()
    : scene_(std::make_unique<Scene>()),
      random_engine_(std::random_device{}())
{
}

Blunder::~Blunder() = default;

void Blunder::RenderImage()
{
    for (int y = 0; y < PixelHeight; ++y) {
        for (int x = 0; x < PixelWidth; ++x) {
            RenderPixel(x, y);
        }
    }
}

void Blunder::RenderPixel(int x, int y)
{
    if (x < 0 || x >= PixelWidth || y < 0 || y >= PixelHeight) {
        throw std::out_of_range("Pixel coordinates are outside the render grid");
    }

    const Camera& camera = scene_->camera;
    const Vector2 pixel_center{static_cast<double>(x) + 0.5, static_cast<double>(y) + 0.5};
    Vector3 final_color = Vector3::zero;
    for (int sample = 0; sample < SamplesPerPixel; ++sample) {
        const double offset_x =
            std::generate_canonical<double, std::numeric_limits<double>::digits>(random_engine_) - 0.5;
        const double offset_y =
            std::generate_canonical<double, std::numeric_limits<double>::digits>(random_engine_) - 0.5;
        const double pixel_x = (pixel_center.x + offset_x) / PixelWidth - 0.5;
        const double pixel_y = (pixel_center.y + offset_y) / PixelHeight - 0.5;
        const Vector3 pixel_position =
            camera.sensor_center()
            + camera.sensor_u() * (pixel_x * camera.sensor_size.x)
            + camera.sensor_v() * (pixel_y * camera.sensor_size.y);

        const Line3 ray{camera.position, pixel_position};
        const std::optional<Hit> hit = closest_hit(*scene_, ray);
        if (!hit) {
            continue;
        }

        const Vector3 hit_point = ray.calc_point(hit->lambda);
        const Vector3 normal = hit->object->normal(hit_point);
        Vector3 sample_color = Vector3::zero;
        for (const std::unique_ptr<Light>& light : scene_->lights) {
            const double intensity = light->intensity(hit_point, normal);
            if (intensity <= 0.0) {
                continue;
            }

            double shadow = 0.0;
            if (light->can_be_eclipsed()) {
                const Vector3 direction_to_light = light->direction_to_light(hit_point);
                const Line3 shadow_ray{hit_point, hit_point + direction_to_light};
                shadow = shadow_attenuation(*scene_, shadow_ray);
            }

            const Vector3 diffuse =
                multiply(hit->object->color(), light->color()) * (intensity * (1.0 - shadow));
            sample_color = sample_color + diffuse;

            if (light->is_specular()) {
                const Vector3 incoming = -1.0 * light->direction_to_light(hit_point);
                const Vector3 reflected = incoming.reflect(normal).normalized();
                const Vector3 view = (camera.position - hit_point).normalized();
                const double specular = std::pow(reflected.dot(view), 7.0);
                if (specular > 0.0) {
                    sample_color = sample_color + light->color() * specular;
                }
            }
        }
        final_color = final_color + sample_color;
    }

    final_color = final_color / SamplesPerPixel;
    pixels_[PixelHeight - y - 1][x] = {
        to_channel(final_color.x),
        to_channel(final_color.y),
        to_channel(final_color.z),
        255};
}

/// Returns this instance's pixel image.
/// @return The generated RGBA pixel grid.
const Blunder::PixelGrid& Blunder::GetPixelGrid() const
{
    return pixels_;
}