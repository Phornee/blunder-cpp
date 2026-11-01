#include <blunder/blunder.hpp>

#include <cassert>
#include <memory>
#include <stdexcept>

int main()
{
    auto blunder = std::make_unique<Blunder>();
    blunder->RenderPixel(Blunder::PixelWidth / 2, Blunder::PixelHeight / 2);

    const Blunder::PixelGrid& pixels = blunder->GetPixelGrid();
    const SDL_Color center = pixels[Blunder::PixelHeight / 2 - 1][Blunder::PixelWidth / 2];
    assert(center.a == 255);
    assert(center.r > 0 || center.g > 0 || center.b > 0);

    blunder->RenderImage();
    const SDL_Color rendered_center =
        blunder->GetPixelGrid()[Blunder::PixelHeight / 2 - 1][Blunder::PixelWidth / 2];
    assert(rendered_center.a == 255);
    assert(rendered_center.r > 0 || rendered_center.g > 0 || rendered_center.b > 0);

    bool rejected_out_of_bounds_pixel = false;
    try {
        blunder->RenderPixel(-1, 0);
    } catch (const std::out_of_range&) {
        rejected_out_of_bounds_pixel = true;
    }
    assert(rejected_out_of_bounds_pixel);
}
