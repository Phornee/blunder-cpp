#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <blunder/camera.hpp>
#include <blunder/light.hpp>
#include <blunder/primitives/object.hpp>
#include <blunder/material.hpp>
#include <geom3/line3.hpp>

class Scene
{
public:
    struct Hit
    {
        double lambda;
        const Object* object;
    };

    Scene();

    Material& add_material(
        std::string name,
        Color color,
        double reflectivity = 0.0,
        std::unique_ptr<Texture> texture = nullptr);
    const std::vector<std::unique_ptr<Material>>&   materials() const;
    std::optional<Hit> closest_hit(const Line3& ray) const;

    Camera camera;

private:
    std::vector<std::unique_ptr<Material>> materials_;

public:
    std::vector<std::unique_ptr<Object>> objects;
    std::vector<std::unique_ptr<Light>> lights;
};
