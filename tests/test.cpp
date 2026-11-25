#include <blunder/blunder.hpp>

#include <cassert>
#include <memory>
#include <stdexcept>

#include <blunder/material.hpp>
#include <blunder/primitives/object.hpp>
#include <blunder/primitives/plane_object.hpp>
#include <blunder/primitives/sphere_object.hpp>
#include <blunder/primitives/triangle_object.hpp>
#include <blunder/scene.hpp>
#include <blunder/texture.hpp>
#include <geom3/sphere.hpp>

int main()
{
    const Material shared_material(Color{0.9, 0.7, 0.7});
    const SphereObject first_object(
        "First",
        shared_material,
        Vector3{0.0, 0.0, 0.0}, 
        1.0);
    const SphereObject second_object(
        "Second",
        shared_material,
        Vector3{2.0, 0.0, 0.0}, 
        1.0);
    assert(&first_object.material() == &second_object.material());
    assert(shared_material.name() == "Material");

    const TriangleObject triangle_object(
        "Triangle",
        shared_material,
        Vector3{-1.0, -1.0, 0.0},
        Vector3{1.0, -1.0, 0.0},
        Vector3{0.0, 1.0, 0.0});
    const Line3 triangle_ray{Vector3{0.0, 0.0, 1.0}, Vector3{0.0, 0.0, 0.0}};
    const std::vector<Scene::Hit> triangle_hits = triangle_object.intersects(triangle_ray);
    assert(triangle_hits.size() == 1);
    assert(triangle_hits.front().object == &triangle_object);
    assert(triangle_hits.front().barycentric.has_value());
    assert(triangle_object.bounding_box().has_value());
    assert((triangle_object.bounding_box()->minimum() == Vector3{-1.0, -1.0, 0.0}));
    assert((triangle_object.bounding_box()->maximum() == Vector3{1.0, 1.0, 0.0}));
    const std::optional<Scene::Hit> triangle_hit =
        triangle_object.closest_intersection(triangle_ray);
    assert(triangle_hit && triangle_hit->lambda > 0.0);
    assert(triangle_hit->object == &triangle_object);
    assert((triangle_hit->normal == Vector3{0.0, 0.0, 1.0}));
    assert(triangle_hit->barycentric.has_value());
    const Color triangle_color = triangle_object.albedo_at(Vector3::zero);
    assert(triangle_color.r() == shared_material.color().r());
    assert(triangle_color.g() == shared_material.color().g());
    assert(triangle_color.b() == shared_material.color().b());

    const Scene scene;
    assert(scene.objects[5]->bounding_box().has_value());
    assert(dynamic_cast<const SphereObject*>(scene.objects[0].get()) != nullptr);
    assert(dynamic_cast<const PlaneObject*>(scene.objects[3].get()) != nullptr);
    assert(dynamic_cast<const TriangleObject*>(scene.objects[4].get()) != nullptr);
    const std::optional<Scene::Hit> closest_hit =
        scene.closest_hit(Line3{Vector3{0.0, 0.0, 10.0}, Vector3{0.0, 0.0, 9.0}});
    assert(closest_hit && closest_hit->object == scene.objects[0].get());
    const Vector3 closest_hit_point =
        Line3{Vector3{0.0, 0.0, 10.0}, Vector3{0.0, 0.0, 9.0}}.calc_point(closest_hit->lambda);
    assert(closest_hit->normal == closest_hit->object->normal_at(closest_hit_point));
    assert(!closest_hit->barycentric);
    const Vector3 triangle_center{2.7666666666666666, -1.6, 0.0};
    const std::optional<Scene::Hit> triangle_scene_hit =
        scene.closest_hit(Line3{scene.camera.position, triangle_center});
    assert(triangle_scene_hit && triangle_scene_hit->object == scene.objects[4].get());
    assert(triangle_scene_hit->barycentric.has_value());
    assert(&scene.objects[0]->material() == scene.materials()[0].get());
    assert(scene.lights[0]->name() == "Omnilight");
    assert(scene.camera.camera_coords_to_world(Vector2{}) == scene.camera.sensor_center());
    const Color floor_color_a = scene.objects[3]->albedo_at(Vector3{0.0, -3.0, 0.0});
    const Color floor_color_b = scene.objects[3]->albedo_at(Vector3{0.5, -3.0, 0.0});
    assert(floor_color_a.r() != floor_color_b.r());

    const Texture jpeg_texture("assets/jpeg_sample.jpg");
    const Color jpeg_color = jpeg_texture.sample(Vector2{});
    assert(jpeg_color.r() > 0.6 && jpeg_color.g() > 0.3 && jpeg_color.b() < 0.35);

    const Material triangle_texture_material(
        "TriangleTexture",
        Color{1.0, 1.0, 1.0},
        0.0,
        std::make_unique<Texture>("assets/jpeg_sample.jpg"));
    const TriangleObject textured_triangle(
        "TexturedTriangle",
        triangle_texture_material,
        Vector3{-1.0, -1.0, 0.0},
        Vector3{1.0, -1.0, 0.0},
        Vector3{0.0, 1.0, 0.0},
        Vector2{0.0, 0.0},
        Vector2{1.0, 0.0},
        Vector2{0.0, 1.0});
    const Color textured_triangle_color = textured_triangle.albedo_at(Vector3{0.25, -0.5, 0.0});
    const Color expected_triangle_color = triangle_texture_material.albedo_at(Vector2{0.5, 0.25});
    assert(textured_triangle_color.r() == expected_triangle_color.r());
    assert(textured_triangle_color.g() == expected_triangle_color.g());
    assert(textured_triangle_color.b() == expected_triangle_color.b());

    Scene named_materials_scene;
    const std::size_t material_count = named_materials_scene.materials().size();
    bool rejected_duplicate_material_name = false;
    try {
        named_materials_scene.add_material("Red", Color{0.1, 0.2, 0.3});
    } catch (const std::invalid_argument&) {
        rejected_duplicate_material_name = true;
    }
    assert(rejected_duplicate_material_name);
    assert(named_materials_scene.materials().size() == material_count);

    auto blunder = std::make_unique<Blunder>();
    const std::optional<Scene::Hit> center_hit =
        blunder->get_hit_info(Blunder::PixelWidth / 2, Blunder::PixelHeight / 2);
    assert(center_hit && center_hit->object->name() == "RedSphere");
    blunder->RenderPixel(Blunder::PixelWidth / 2, Blunder::PixelHeight / 2);

    const Blunder::PixelGrid& pixels = blunder->GetPixelGrid();
    const SDL_Color center = pixels[Blunder::PixelHeight / 2 - 1][Blunder::PixelWidth / 2];
    assert(center.a == 255);
    assert(center.r > 0 || center.g > 0 || center.b > 0);

    blunder->RenderImage();
    const SDL_Color rendered_center =
        blunder->GetPixelGrid()[Blunder::PixelHeight / 2 - 1][Blunder::PixelWidth / 2];
    assert(rendered_center.a == 255);
    assert(rendered_center.r > 0 || rendered_center.g > 0 || rendered_center.b > 0);
    for (const auto& row : blunder->GetPixelGrid()) {
        for (const SDL_Color& pixel : row) {
            assert(pixel.a == 255);
        }
    }

    bool rejected_out_of_bounds_pixel = false;
    try {
        blunder->RenderPixel(-1, 0);
    } catch (const std::out_of_range&) {
        rejected_out_of_bounds_pixel = true;
    }
    assert(rejected_out_of_bounds_pixel);
}
