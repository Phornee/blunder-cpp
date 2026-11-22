#include <blunder/blunder.hpp>

#include <cassert>
#include <memory>
#include <stdexcept>

#include <blunder/material.hpp>
#include <blunder/raytraceable.hpp>
#include <blunder/scene.hpp>
#include <geom3/sphere.hpp>

int main()
{
    const Material shared_material(Color{0.9, 0.7, 0.7});
    const Raytraceable first_object(
        "First",
        shared_material,
        std::make_unique<Sphere>(Vector3{0.0, 0.0, 0.0}, 1.0));
    const Raytraceable second_object(
        "Second",
        shared_material,
        std::make_unique<Sphere>(Vector3{2.0, 0.0, 0.0}, 1.0));
    assert(&first_object.material() == &second_object.material());
    assert(&first_object.color() == &shared_material.color());
    assert(shared_material.name() == "Material");

    const Scene scene;
    assert(&scene.objects[0].material() == scene.materials()[0].get());
    assert(scene.lights[0]->name() == "Omnilight");

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

    bool rejected_out_of_bounds_pixel = false;
    try {
        blunder->RenderPixel(-1, 0);
    } catch (const std::out_of_range&) {
        rejected_out_of_bounds_pixel = true;
    }
    assert(rejected_out_of_bounds_pixel);
}
