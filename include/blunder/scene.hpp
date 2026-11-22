#pragma once

#include <memory>
#include <string>
#include <vector>

#include <blunder/camera.hpp>
#include <blunder/light.hpp>
#include <blunder/raytraceable.hpp>
#include <blunder/material.hpp>

class Scene
{
public:
    Scene();

    Material&                                       add_material(std::string name, Color color, double reflectivity = 0.0);
    const std::vector<std::unique_ptr<Material>>&   materials() const;

    Camera camera;

private:
    std::vector<std::unique_ptr<Material>> materials_;

public:
    std::vector<std::unique_ptr<Raytraceable>> objects;
    std::vector<std::unique_ptr<Light>> lights;
};
