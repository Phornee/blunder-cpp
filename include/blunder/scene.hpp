#pragma once

#include <memory>
#include <vector>

#include <blunder/camera.hpp>
#include <blunder/light.hpp>
#include <blunder/raytraceable.hpp>

class Scene
{
public:
    Scene();

    Camera camera;
    std::vector<Raytraceable> objects;
    std::vector<std::unique_ptr<Light>> lights;
};
