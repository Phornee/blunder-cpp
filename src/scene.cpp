#include <blunder/scene.hpp>

#include <memory>
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
    lights.push_back(std::make_unique<AmbientLight>(Vector3{0.0, 0.0, 0.2}, "Omnilight"));
    lights.push_back(std::make_unique<PointLight>(
        Vector3{0.5, 1.0, 0.5},
        "Green light",
        Vector3{-3.5, 0.0, 2.0}));
    lights.push_back(std::make_unique<PointLight>(
        Vector3{1.0, 0.5, 0.5},
        "Red light",
        Vector3{3.5, 4.0, -2.0}));

    objects.emplace_back(
        "Red sphere",
        Vector3{0.9, 0.7, 0.7},
        std::make_unique<Sphere>(Vector3{0.0, 0.0, -5.0}, 3.0));
    objects.emplace_back(
        "Floor",
        Vector3{0.9, 0.9, 0.9},
        std::make_unique<Plane>(Vector3{0.0, -3.0, 0.0}, Vector3{0.0, 1.0, 0.0}));
}
