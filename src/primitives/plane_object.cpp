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
          std::make_unique<Plane>(point, normal))
{
}

void PlaneObject::transform_shape() const
{
    Plane* plane = dynamic_cast<Plane*>(shape_.get());
    Vector3 transformed_point = transform_ * plane->a;
    Vector3 transformed_normal = (transform_ * (plane->a + plane->n)) - transformed_point;

    if (transformed_shape_ == nullptr) {
        transformed_shape_ = std::make_unique<Plane>(transformed_point, transformed_normal);
    }
    else
    {
        static_cast<Plane*>(transformed_shape_.get())->a = transformed_point;
        static_cast<Plane*>(transformed_shape_.get())->n = transformed_normal;
    }
}

Color PlaneObject::color_at(const Vector3& point) const
{
    const std::optional<Vector2> coordinates = shape_->texture_coordinates(point, 0.1);
    return coordinates ? material_.get().color_at(*coordinates) : material_.get().color();
}
