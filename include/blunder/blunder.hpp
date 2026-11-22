#pragma once

#include <array>
#include <memory>
#include <random>

#include <SDL3/SDL.h>

#include <blunder/color.hpp>

class Scene;
class Light;
class Raytraceable;
class Vector3;
class Line3;

/// Owns the state and pixel data for the Blunder application.
class Blunder{
public:
    static constexpr int PixelWidth = 400;
    static constexpr int PixelHeight = 300;
    using PixelGrid = std::array<std::array<SDL_Color, PixelWidth>, PixelHeight>;

    /// Creates a Blunder instance with the default ray-tracing scene.
                        Blunder();
                        ~Blunder();

    /// Renders the complete scene into the pixel grid.
    void                RenderImage();

    /// Renders one pixel in window coordinates.
    void                RenderPixel(int x, int y);

    /// Provides read-only access to the rendered pixel grid.
    /// @return The fixed-size RGBA pixel grid owned by this instance.
    const PixelGrid&    GetPixelGrid() const;

private:
    Color light_contribution(
        const Raytraceable& object,
        const Light& light,
        const Vector3& hit_point,
        const Vector3& normal) const;

    Color               get_incomming_light(const Scene& scene, const Line3& ray, int depth = 0);

    std::unique_ptr<Scene>  scene_;
    PixelGrid               pixels_{};
    std::mt19937            random_engine_;
};