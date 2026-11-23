#include <blunder/primitives/object.hpp>

#include <stdexcept>
#include <utility>

Object::Object(std::string name, const Material& material, std::unique_ptr<Shape> shape)
    : name_(std::move(name)), material_(material), shape_(std::move(shape))
{
    if (!shape_) {
        throw std::invalid_argument("An object must have a shape");
    }
}

const std::string& Object::name() const
{
    return name_;
}

const Material& Object::material() const
{
    return material_.get();
}

const Color& Object::color() const
{
    return material_.get().color();
}

const Shape* Object::transformed_shape() const 
{ 
    if (transformed_shape_ == nullptr) {
        transform_shape();
    }
    return transformed_shape_.get();
}

Color Object::color_at(const Vector3& point) const
{
    return Color(1.0, 0.0, 1.0);
}

Vector3 Object::normal(const Vector3& point) const
{
    return transformed_shape()->normal(point);
}

std::optional<double> Object::closest_intersection(const Line3& line) const
{
    return transformed_shape()->closest_intersection(line);
}

void Object::apply_transform(const Matrix4& transform)
{
    transform_ = transform_ * transform;
}
