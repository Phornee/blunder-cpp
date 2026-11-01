#include <mini_font/mini_font.hpp>

#include <array>
#include <cstdint>

namespace
{
/// Returns the seven 5-bit rows that encode a supported font character.
/// @param character Character to look up in the bitmap font.
/// @return One 5-pixel-wide bitmap row per glyph row.
std::array<std::uint8_t, 7> GetGlyphRows(char character)
{
    switch (character)
    {
    case '0': return {0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E};
    case '1': return {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E};
    case '2': return {0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F};
    case '3': return {0x1E, 0x01, 0x01, 0x0E, 0x01, 0x01, 0x1E};
    case '4': return {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02};
    case '5': return {0x1F, 0x10, 0x10, 0x1E, 0x01, 0x01, 0x1E};
    case '6': return {0x0E, 0x10, 0x10, 0x1E, 0x11, 0x11, 0x0E};
    case '7': return {0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08};
    case '8': return {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E};
    case '9': return {0x0E, 0x11, 0x11, 0x0F, 0x01, 0x01, 0x0E};
    case 'F': return {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10};
    case 'P': return {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10};
    case 'S': return {0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E};
    case ':': return {0x00, 0x04, 0x04, 0x00, 0x04, 0x04, 0x00};
    default: return {};
    }
}
}

/// Draws each set bit in the glyph rows as a scaled rectangle.
bool MiniFont::DrawText(
    SDL_Renderer* renderer,
    std::string_view text,
    int x,
    int y,
    int scale,
    SDL_Color color) const
{
    if (scale <= 0 || !SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a))
    {
        return false;
    }

    int characterX = x;
    for (const char character : text)
    {
        const auto rows = GetGlyphRows(character);
        for (int row = 0; row < static_cast<int>(rows.size()); ++row)
        {
            for (int column = 0; column < 5; ++column)
            {
                if ((rows[row] & (1U << (4 - column))) == 0)
                {
                    continue;
                }

                const SDL_FRect pixel = {
                    static_cast<float>(characterX + column * scale),
                    static_cast<float>(y + row * scale),
                    static_cast<float>(scale),
                    static_cast<float>(scale)};
                if (!SDL_RenderFillRect(renderer, &pixel))
                {
                    return false;
                }
            }
        }
        characterX += 6 * scale;
    }

    return true;
}
