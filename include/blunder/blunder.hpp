#pragma once

#include <array>
#include <memory>
#include <random>

#include <SDL3/SDL.h>

class Scene;

/// Owns the state and pixel data for the Blunder application.
class Blunder
{
public:
    static constexpr int PixelWidth = 800;
    static constexpr int PixelHeight = 600;
    using PixelGrid = std::array<std::array<SDL_Color, PixelWidth>, PixelHeight>;

    /// Creates a Blunder instance with the default ray-tracing scene.
    Blunder();
    ~Blunder();

    /// Renders the complete scene into the pixel grid.
    void RenderImage();

    /// Renders one pixel in window coordinates.
    void RenderPixel(int x, int y);

    /// Provides read-only access to the rendered pixel grid.
    /// @return The fixed-size RGBA pixel grid owned by this instance.
    const PixelGrid& GetPixelGrid() const;

private:
    std::unique_ptr<Scene> scene_;
    PixelGrid pixels_{};
    std::mt19937 random_engine_;
};