#include <blunder/blunder.hpp>

#include <cstdint>

/// Initializes this instance's gradient and checkerboard pixel image.
Blunder::Blunder()
{
    for (int y = 0; y < Blunder::PixelHeight; ++y)
    {
        for (int x = 0; x < Blunder::PixelWidth; ++x)
        {
            const auto red = static_cast<std::uint8_t>(x * 255 / (Blunder::PixelWidth - 1));
            const auto green = static_cast<std::uint8_t>(y * 255 / (Blunder::PixelHeight - 1));
            const auto blue = static_cast<std::uint8_t>(
                ((x / 32 + y / 32) % 2 == 0) ? 220 : 70);
            pixels_[y][x] = {red, green, blue, 255};
        }
    }
}

/// Returns this instance's pixel image.
/// @return The generated RGBA pixel grid.
const Blunder::PixelGrid& Blunder::GetPixelGrid() const
{
    return pixels_;
}