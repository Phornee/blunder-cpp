#pragma once

#include <string>
#include <utility>

#include <blunder/color.hpp>

class Material
{
public:
    explicit Material(Color color, double reflectivity = 0.0)
        : Material("Material", color, reflectivity)
    {
    }

    Material(std::string name, Color color, double reflectivity = 0.0)
        : name_(std::move(name)), color_(color), reflectivity_(reflectivity)
    {
    }

    virtual ~Material() = default;

    const std::string& name() const
    {
        return name_;
    }

    const Color& color() const
    {
        return color_;
    }

    double reflectivity() const
    {
        return reflectivity_;
    }

private:
    std::string         name_;
    Color               color_;
    double              reflectivity_;
};