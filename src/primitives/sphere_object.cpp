#include <blunder/primitives/sphere_object.hpp>

#include <utility>

#include <geom3/sphere.hpp>

SphereObject::SphereObject(
    std::string name,
    const Material& material,
    const Vector3& center,
    double radius)
    : Object(
          std::move(name),
          material,
          std::make_unique<Sphere>(Vector3(0, 0, 0), radius))
{
    transform_ = Matrix4::translation(center);
    transformed_shape_ = std::make_unique<Sphere>(center, radius);
    bounding_box_ = std::nullopt;
}

void SphereObject::transform_shape() const
{
    Sphere* sphere = dynamic_cast<Sphere*>(shape_.get());
    Vector3 transformed_center = transform_ * sphere->c;
    double transformed_radius = sphere->r * transform_.scaling_factor();

    static_cast<Sphere*>(transformed_shape_.get())->c = transformed_center;
    static_cast<Sphere*>(transformed_shape_.get())->r = transformed_radius;
    const Vector3 extent{transformed_radius, transformed_radius, transformed_radius};
}

Color SphereObject::albedo_at(const Vector3&) const
{
    return material_.get().color();
}
