#include <blunder/raytraceable.hpp>

#include <stdexcept>
#include <utility>

Raytraceable::Raytraceable(std::string name, const Material& material, std::unique_ptr<Shape> shape)
    : name_(std::move(name)), material_(material), shape_(std::move(shape))
{
    if (!shape_) {
        throw std::invalid_argument("A raytraceable object must have a shape");
    }
}

const std::string& Raytraceable::name() const
{
    return name_;
}

const Material& Raytraceable::material() const
{
    return material_.get();
}

const Color& Raytraceable::color() const
{
    return material_.get().color();
}

Vector3 Raytraceable::normal(const Vector3& point) const
{
    return shape_->normal(point);
}

std::optional<double> Raytraceable::closest_intersection(const Line3& line) const
{
    return shape_->closest_intersection(line);
}
