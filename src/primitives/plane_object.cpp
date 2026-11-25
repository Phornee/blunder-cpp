#include <blunder/primitives/plane_object.hpp>

#include <utility>

#include <geom3/plane.hpp>

PlaneObject::PlaneObject(
    std::string name,
    const Material& material,
    const Vector3& point,
    const Vector3& normal)
    : Object(
          std::move(name),
          material,
          std::make_unique<Plane>(Vector3::zero, normal))
{
    transform_ = Matrix4::translation(point);
    transform_shape();
    bounding_box_ = std::nullopt;
}

void PlaneObject::transform_shape() const
{
    Plane* plane = dynamic_cast<Plane*>(shape_.get());
    Vector3 transformed_point = transform_ * plane->a;
    Vector3 transformed_normal = transform_.transform_direction(plane->n);
    transformed_shape_ = std::make_unique<Plane>(transformed_point, transformed_normal);
}

Color PlaneObject::albedo_at(const Vector3& point) const
{
    const std::optional<Vector2> coordinates =
        transformed_shape()->texture_coordinates(point, 0.1);
    return coordinates ? material_.get().albedo_at(*coordinates) : material_.get().color();
}
