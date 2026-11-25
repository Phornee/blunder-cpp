#pragma once

#include <array>
#include <memory>
#include <optional>

#include <SDL3/SDL.h>

#include <blunder/color.hpp>
#include <blunder/scene.hpp>

class Light;
class Object;
class Vector3;
class Vector2;
class Line3;

/// Owns the state and pixel data for the Blunder application.
class Blunder{
public:
    static constexpr int PixelWidth = 800;
    static constexpr int PixelHeight = 600;
    using PixelGrid = std::array<std::array<SDL_Color, PixelWidth>, PixelHeight>;

    /// Creates a Blunder instance with the default ray-tracing scene.
                        Blunder();
                        ~Blunder();

    /// Renders the complete scene into the pixel grid.
    void                RenderImage();

    /// Renders one pixel in window coordinates.
    void                RenderPixel(int x, int y);

    /// Returns the closest hit along the center ray for a pixel.
    /// @return The nearest scene hit, or std::nullopt if the ray misses.
    std::optional<Hit> get_hit_info(int x, int y) const;

    /// Provides read-only access to the rendered pixel grid.
    /// @return The fixed-size RGBA pixel grid owned by this instance.
    const PixelGrid&    GetPixelGrid() const;

private:
    Color light_contribution(
        const Color& surface_color,
        const Light& light,
        const Vector3& hit_point,
        const Vector3& normal) const;

    Vector2             screen_coords_to_camera(int x, int y) const;
    Color               get_incomming_light(const Scene& scene, const Line3& ray);
    Color               trace_ray(const Scene& scene, const Line3& ray, int depth);

    std::unique_ptr<Scene>  scene_;
    PixelGrid               pixels_{};
};