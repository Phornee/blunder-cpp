#include <blunder/texture.hpp>

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include <algorithm>
#include <cmath>
#include <memory>
#include <stdexcept>

Texture::Texture(const std::string& image_path)
    : width_(0), height_(0)
{
    using Surface = std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)>;
    Surface loaded_surface(IMG_Load(image_path.c_str()), SDL_DestroySurface);
    if (!loaded_surface) {
        throw std::runtime_error(
            "Failed to load texture '" + image_path + "': " + SDL_GetError());
    }

    Surface rgba_surface(
        SDL_ConvertSurface(loaded_surface.get(), SDL_PIXELFORMAT_RGBA32),
        SDL_DestroySurface);
    if (!rgba_surface) {
        throw std::runtime_error(
            "Failed to convert texture '" + image_path + "': " + SDL_GetError());
    }

    width_ = rgba_surface->w;
    height_ = rgba_surface->h;
    if (width_ <= 0 || height_ <= 0) {
        throw std::runtime_error("Texture has invalid dimensions: '" + image_path + "'");
    }
    pixels_.resize(static_cast<std::size_t>(width_) * height_);

    const bool must_unlock = SDL_MUSTLOCK(rgba_surface.get());
    if (must_unlock && !SDL_LockSurface(rgba_surface.get())) {
        throw std::runtime_error(
            "Failed to lock texture '" + image_path + "': " + SDL_GetError());
    }

    const auto* source_pixels = static_cast<const Uint8*>(rgba_surface->pixels);
    for (int y = 0; y < height_; ++y) {
        const Uint8* row = source_pixels + y * rgba_surface->pitch;
        for (int x = 0; x < width_; ++x) {
            const Uint8* pixel = row + x * 4;
            pixels_[static_cast<std::size_t>(y) * width_ + x] = {
                pixel[0] / 255.0,
                pixel[1] / 255.0,
                pixel[2] / 255.0};
        }
    }

    if (must_unlock) {
        SDL_UnlockSurface(rgba_surface.get());
    }
}

Color Texture::sample(const Vector2& coordinates) const
{
    const double wrapped_u = coordinates.x - std::floor(coordinates.x);
    const double wrapped_v = coordinates.y - std::floor(coordinates.y);
    const int x = std::min(static_cast<int>(wrapped_u * width_), width_ - 1);
    const int y = std::min(static_cast<int>((1.0 - wrapped_v) * height_), height_ - 1);
    return pixels_[static_cast<std::size_t>(y) * width_ + x];
}
