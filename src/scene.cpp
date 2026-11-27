#include <blunder/scene.hpp>

#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>

#include <blunder/primitives/mesh_object.hpp>
#include <blunder/primitives/plane_object.hpp>
#include <blunder/primitives/sphere_object.hpp>
#include <blunder/primitives/triangle_object.hpp>

Scene::Scene()
    : camera(Vector3{0.0, 0.0, 10.0}, Vector3{0.0, 0.0, -1.0}, Vector2{0.053, 0.0402}, 0.05)
{
    Material& red_mat = add_material("Red", Color{0.9, 0.5, 0.5}, 0.5);
    Material& blue_mat = add_material("Blue", Color{0.5, 0.5, 0.9}, 0.5);
    Material& green_mat = add_material("Green", Color{0.2, 0.9, 0.2}, 0.5);
    Material& yellow_mat = add_material("Yellow", Color{0.9, 0.8, 0.2}, 0.1, std::make_unique<Texture>("assets/beige-limestone-tiles-2k.jpg"));
    Material& white_mat = add_material("White", Color{0.9, 0.9, 0.9}, 0.1, std::make_unique<Texture>("assets/beige-limestone-tiles-2k.jpg"));

    lights.push_back(std::make_unique<AmbientLight>("Omnilight", Color{0.0, 0.0, 0.2}));
    lights.push_back(std::make_unique<PointLight>("WhiteLight", Color{1.0, 1.0, 1.0}, Vector3{-3.5, 0.0, 2.0}, 0.5));
    lights.push_back(std::make_unique<PointLight>("RedLight", Color{1.0, 0.7, 0.7}, Vector3{3.5, 4.0, -2.0}, 0.5));
    objects.push_back(std::make_unique<SphereObject>("RedSphere", red_mat, Vector3{0.0, 0.0, -5.0}, 3.0));
    objects.push_back(std::make_unique<SphereObject>("LeftSphere", blue_mat, Vector3{3.0, 2.0, 0.0}, 0.5));
    objects.push_back(std::make_unique<SphereObject>("RightSphere", green_mat, Vector3{-2.0, 1.0, 5.0}, 0.5));
    objects.push_back(std::make_unique<PlaneObject>("Floor", white_mat, Vector3{0.0, -3.0, 0.0}, Vector3{0.0, 1.0, 0.0}));
    objects.push_back(std::make_unique<TriangleObject>(
        "YellowTriangle", yellow_mat, Vector3{1.5, -2.4, 0.0}, Vector3{4.0, -2.4, 0.0}, Vector3{2.8, 0.0, 0.0}));
    auto cube = std::make_unique<MeshObject>("Cube", green_mat, "assets/mahouBottle.obj");
    Matrix4 cube_transform =    Matrix4::scaling(Vector3{0.5, 0.5, 0.5}) *
                                Matrix4::translation(Vector3{-1.0, -1.3, 3.0}) *
                                Matrix4::rotation_y(45.0);
                                
    cube->set_transform(cube_transform);
    objects.push_back(std::move(cube));
}

Material& Scene::add_material(
    std::string name,
    Color color,
    double reflectivity,
    std::unique_ptr<Texture> texture)
{
    for (const std::unique_ptr<Material>& material : materials_) {
        if (material->name() == name) {
            throw std::invalid_argument("A material with this name already exists in the scene: " + name);
        }
    }

    std::unique_ptr<Material> material =
        std::make_unique<Material>(
            std::move(name),
            color,
            reflectivity,
            std::move(texture));
    Material& result = *material;
    materials_.push_back(std::move(material));
    return result;
}

const std::vector<std::unique_ptr<Material>>& Scene::materials() const
{
    return materials_;
}

void Scene::transform_objects()
{
    for (const std::unique_ptr<Object>& object : objects) {
        object->transformed_shape();
    }
}

std::optional<Hit> Scene::closest_hit(const Line3& ray) const
{
    std::optional<Hit> closest;
    double closest_lambda = std::numeric_limits<double>::max();
    for (const std::unique_ptr<Object>& object : objects)
    {
        //if (!object->bounding_box() || object->bounding_box()->intersects(ray)) {
            const std::optional<Hit> intersection = object->closest_intersection(ray);
            if (intersection && intersection->lambda > epsilon && intersection->lambda < closest_lambda)
            {
                closest_lambda = intersection->lambda;
                closest = intersection;
            }
        //}
    }
    return closest;
}
