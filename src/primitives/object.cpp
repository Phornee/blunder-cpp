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

// const Color& Object::color() const
// {
//     return material_.get().color();
// }

Color Object::albedo_at(const Vector3& point) const
{
    return Color(1.0, 0.0, 1.0);
}

Vector3 Object::normal_at(const Vector3& point) const
{
    return transformed_shape()->normal(point);
}

std::vector<Hit> Object::intersects(const Line3& line) const
{
    std::vector<Hit> hits = transformed_shape()->intersects(line);
    for (Hit& hit : hits) {
        hit.object = this;
    }
    return hits;
}

std::optional<Hit> Object::closest_intersection(const Line3& line) const
{
    std::optional<Hit> hit = transformed_shape()->closest_intersection(line);
    if (hit) {
        hit->object = this;
    }
    return hit;
}

const Shape* Object::transformed_shape() const
{
    if (transformed_shape_dirty_) {
        transform_shape();
        transformed_shape_dirty_ = false;
    }
    return transformed_shape_.get();
}
