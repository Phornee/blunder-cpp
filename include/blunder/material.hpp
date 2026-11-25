#pragma once

#include <memory>
#include <string>
#include <utility>

#include <blunder/color.hpp>
#include <blunder/texture.hpp>

class Material
{
public:
    explicit Material(Color color, double reflectivity = 0.0)
        : Material("Material", color, reflectivity)
    {
    }

    Material(
        std::string name,
        Color color,
        double reflectivity = 0.0,
        std::unique_ptr<Texture> texture = nullptr)
        : name_(std::move(name)),
          color_(color),
          reflectivity_(reflectivity),
          color_texture_(std::move(texture))
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

    Color albedo_at(const Vector2& coordinates) const
    {
        return color_texture_ ? color_texture_->sample(coordinates) * color_ : color_;
    }

private:
    std::string                 name_;
    Color                       color_;
    double                      reflectivity_;
    std::unique_ptr<Texture>    color_texture_;
};