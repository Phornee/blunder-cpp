#include <blunder/scene.hpp>

#include <memory>
#include <stdexcept>
#include <utility>

#include <geom3/plane.hpp>
#include <geom3/sphere.hpp>

Scene::Scene()
    : camera(
          Vector3{0.0, 0.0, 10.0},
          Vector3{0.0, 0.0, -1.0},
          Vector2{0.053, 0.0402},
          0.05)
{
    Material& red_mat = add_material("Red", Color{0.9, 0.7, 0.7}, 0.5);
    Material& blue_mat = add_material("Blue", Color{0.7, 0.7, 0.9}, 0.5);
    Material& white_mat = add_material("White", Color{0.9, 0.9, 0.9}, 0.8);

    lights.push_back(std::make_unique<AmbientLight>("Omnilight", Color{0.0, 0.0, 0.2}));
    lights.push_back(std::make_unique<PointLight>(
        "WhiteLight",
        Color{1.0, 1.0, 1.0},
        Vector3{-3.5, 0.0, 2.0}));
    lights.push_back(std::make_unique<PointLight>(
        "RedLight",
        Color{1.0, 0.5, 0.5},
        Vector3{3.5, 4.0, -2.0}));

    objects.push_back(std::make_unique<Raytraceable>(
        "RedSphere",
        red_mat,
        std::make_unique<Sphere>(Vector3{0.0, 0.0, -5.0}, 3.0)));
    objects.push_back(std::make_unique<Raytraceable>(
        "BlueSphere",
        blue_mat,
        std::make_unique<Sphere>(Vector3{3.0, 2.0, 0.0}, 0.5)));
    objects.push_back(std::make_unique<Raytraceable>(
        "Floor",
        white_mat,
        std::make_unique<Plane>(Vector3{0.0, -3.0, 0.0}, Vector3{0.0, 1.0, 0.0})));
}

Material& Scene::add_material(std::string name, Color color, double reflectivity)
{
    for (const std::unique_ptr<Material>& material : materials_) {
        if (material->name() == name) {
            throw std::invalid_argument("A material with this name already exists in the scene: " + name);
        }
    }

    std::unique_ptr<Material> material =
        std::make_unique<Material>(std::move(name), color, reflectivity);
    Material& result = *material;
    materials_.push_back(std::move(material));
    return result;
}

const std::vector<std::unique_ptr<Material>>& Scene::materials() const
{
    return materials_;
}
