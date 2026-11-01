#pragma once

#include <string_view>

#include <SDL3/SDL.h>

/// Draws text using a compact built-in 5-by-7 pixel font.
class MiniFont
{
public:
    /// Draws text at the specified position and scale.
    /// Unsupported characters render as blank glyphs.
    /// @param renderer SDL renderer used to draw the text.
    /// @param text Characters to draw.
    /// @param x Left coordinate of the first glyph.
    /// @param y Top coordinate of the glyphs.
    /// @param scale Size multiplier for each font pixel; must be positive.
    /// @param color Color used to draw the font pixels.
    /// @return true if drawing succeeded; false for an invalid scale or SDL error.
    bool DrawText(
        SDL_Renderer* renderer,
        std::string_view text,
        int x,
        int y,
        int scale,
        SDL_Color color) const;
};
