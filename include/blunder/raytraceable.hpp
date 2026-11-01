#pragma once

#include <memory>
#include <string>

#include <geom3/base.hpp>

class Raytraceable
{
public:
    Raytraceable(std::string name, Vector3 color, std::unique_ptr<Shape> shape);

    const std::string& name() const;
    const Vector3& color() const;
    Vector3 normal(const Vector3& point) const;
    std::optional<double> closest_intersection(const Line3& line) const;

private:
    std::string name_;
    Vector3 color_;
    std::unique_ptr<Shape> shape_;
};
