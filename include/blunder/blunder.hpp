#pragma once

#include <array>

#include <SDL3/SDL.h>

/// Owns the state and pixel data for the Blunder application.
class Blunder
{
public:
    static constexpr int PixelWidth = 800;
    static constexpr int PixelHeight = 600;
    using PixelGrid = std::array<std::array<SDL_Color, PixelWidth>, PixelHeight>;

    /// Creates a Blunder instance with its initial pixel image.
    Blunder();

    /// Provides read-only access to this instance's pixel image.
    /// @return The fixed-size RGBA pixel grid owned by this instance.
    const PixelGrid& GetPixelGrid() const;

private:
    PixelGrid pixels_;
};