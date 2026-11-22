#pragma once

#include <functional>
#include <memory>
#include <string>

#include <geom3/base.hpp>
#include <blunder/material.hpp>

class Raytraceable
{
public:
                            Raytraceable(std::string name, const Material& material, std::unique_ptr<Shape> shape);

    const std::string&      name() const;
    const Material&         material() const;
    const Color&            color() const;
    Vector3                 normal(const Vector3& point) const;
    std::optional<double>   closest_intersection(const Line3& line) const;

private:
    std::string                             name_;
    std::reference_wrapper<const Material>  material_;
    std::unique_ptr<Shape>                  shape_;
};
