#include <chrono>
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <string>

#include <SDL3/SDL.h>

#include <blunder/blunder.hpp>
#include <mini_font/mini_font.hpp>
#include "geom3/geom3.hpp"

namespace
{
/// Draws the FPS label and value over the current frame.
/// @param renderer SDL renderer used to draw the HUD.
/// @param fps Frame rate value to display.
/// @param miniFont Font used to draw the HUD label.
/// @return true if all HUD drawing commands succeeded.
bool DrawFpsHud(
    SDL_Renderer* renderer,
    int fps,
    int mouseX,
    int mouseY,
    const MiniFont& miniFont)
{
    constexpr int scale = 1;
    constexpr int padding = 8;
    const std::string fpsText = "FPS: " + std::to_string(fps);
    const std::string mouseText = "(" + std::to_string(mouseX) + ", " + std::to_string(mouseY) + ")";
    const int textWidth = std::max(
        static_cast<int>(fpsText.size()),
        static_cast<int>(mouseText.size())) * 6 - 1;
    const SDL_FRect background = {
        static_cast<float>(::Blunder::PixelWidth - textWidth - 2 * padding - 12),
        12.0F,
        static_cast<float>(textWidth + 2 * padding),
        15.0F * scale + 2 * padding};

    if (!SDL_SetRenderDrawColor(renderer, 10, 14, 20, 255) ||
        !SDL_RenderFillRect(renderer, &background))
    {
        return false;
    }

    return miniFont.DrawText(
        renderer,
        fpsText,
        static_cast<int>(background.x) + padding,
        static_cast<int>(background.y) + padding,
        scale,
        {255, 255, 255, 255}) &&
        miniFont.DrawText(
            renderer,
            mouseText,
            static_cast<int>(background.x) + padding,
            static_cast<int>(background.y) + padding + 8,
            scale,
            {255, 255, 255, 255});
}

bool GetMousePixel(SDL_Renderer* renderer, int& x, int& y)
{
    float windowX = 0.0F;
    float windowY = 0.0F;
    float logicalX = 0.0F;
    float logicalY = 0.0F;
    SDL_GetMouseState(&windowX, &windowY);
    if (!SDL_RenderCoordinatesFromWindow(renderer, windowX, windowY, &logicalX, &logicalY))
    {
        return false;
    }

    x = std::clamp(static_cast<int>(logicalX), 0, ::Blunder::PixelWidth - 1);
    y = std::clamp(static_cast<int>(logicalY), 0, ::Blunder::PixelHeight - 1);
    return true;
}

/// Uploads the pixel grid, draws it, overlays the FPS HUD, and presents the frame.
/// @param renderer SDL renderer used to present the scene.
/// @param pixelTexture Streaming texture that receives the pixel grid.
/// @param fps Frame rate value passed to the HUD.
/// @return true if the texture update and all rendering commands succeeded.
bool RenderFrame(
    SDL_Renderer* renderer,
    SDL_Texture* pixelTexture,
    const ::Blunder& blunder,
    const MiniFont& miniFont,
    int fps,
    int mouseX,
    int mouseY)
{
    const Blunder::PixelGrid& pixels = blunder.GetPixelGrid();

    return SDL_UpdateTexture(
            pixelTexture,
            nullptr,
            pixels.data(),
            static_cast<int>(sizeof(pixels[0]))) &&
        SDL_RenderClear(renderer) &&
        SDL_RenderTexture(renderer, pixelTexture, nullptr, nullptr) &&
        DrawFpsHud(renderer, fps, mouseX, mouseY, miniFont) &&
        SDL_RenderPresent(renderer);
}
}

/// Runs the ray tracer until the window is closed or Escape is pressed.
/// Initializes SDL resources, renders the scene, processes events, and cleans up.
/// @return 0 on normal exit, or 1 if SDL initialization or rendering fails.
int main()
{
    ::Blunder blunder;
    MiniFont miniFont;

    std::cout << "Press D to rerender the pixel under the mouse, or Escape to exit.\n";

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << '\n';
        return 1;
    }

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    if (!SDL_CreateWindowAndRenderer(
            "Blunder - SDL3",
            ::Blunder::PixelWidth,
            ::Blunder::PixelHeight,
            SDL_WINDOW_RESIZABLE,
            &window,
            &renderer))
    {
        std::cerr << "SDL_CreateWindowAndRenderer failed: " << SDL_GetError() << '\n';
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    if (!SDL_SetRenderLogicalPresentation(
            renderer,
            ::Blunder::PixelWidth,
            ::Blunder::PixelHeight,
            SDL_LOGICAL_PRESENTATION_LETTERBOX))
    {
        std::cerr << "SDL_SetRenderLogicalPresentation failed: " << SDL_GetError() << '\n';
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Texture* pixelTexture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA32,
        SDL_TEXTUREACCESS_STREAMING,
        ::Blunder::PixelWidth,
        ::Blunder::PixelHeight);
    if (pixelTexture == nullptr)
    {
        std::cerr << "SDL_CreateTexture failed: " << SDL_GetError() << '\n';
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    if (!SDL_SetTextureScaleMode(pixelTexture, SDL_SCALEMODE_NEAREST))
    {
        std::cerr << "SDL_SetTextureScaleMode failed: " << SDL_GetError() << '\n';
        SDL_DestroyTexture(pixelTexture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    std::uint64_t totalRenderMilliseconds = 0;
    unsigned int renderedImages = 0;
    const auto renderStart = std::chrono::steady_clock::now();
    blunder.RenderImage();
    totalRenderMilliseconds += static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - renderStart).count());
    ++renderedImages;

    bool running = true;
    int exitCode = 0;
    int displayedFps = 0;
    unsigned int frameCount = 0;
    auto fpsWindowStart = std::chrono::steady_clock::now();
    SDL_Event event;
    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT ||
                (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE))
            {
                running = false;
            }
            else if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_D)
            {
                int mouseX = 0;
                int mouseY = 0;
                if (!GetMousePixel(renderer, mouseX, mouseY))
                {
                    std::cerr << "SDL_RenderCoordinatesFromWindow failed: " << SDL_GetError() << '\n';
                    exitCode = 1;
                    running = false;
                    break;
                }
                blunder.RenderPixel(mouseX, mouseY);
            }
        }

        if (!running)
        {
            break;
        }

        int mouseX = 0;
        int mouseY = 0;
        if (!GetMousePixel(renderer, mouseX, mouseY))
        {
            std::cerr << "SDL_RenderCoordinatesFromWindow failed: " << SDL_GetError() << '\n';
            exitCode = 1;
            break;
        }

        if (!RenderFrame(renderer, pixelTexture, blunder, miniFont, displayedFps, mouseX, mouseY))
        {
            std::cerr << "SDL rendering failed: " << SDL_GetError() << '\n';
            exitCode = 1;
            break;
        }

        ++frameCount;
        const auto now = std::chrono::steady_clock::now();
        const std::chrono::duration<double> elapsed = now - fpsWindowStart;
        if (elapsed.count() >= 1.0)
        {
            displayedFps = static_cast<int>(frameCount / elapsed.count() + 0.5);
            frameCount = 0;
            fpsWindowStart = now;
        }
        SDL_Delay(16);
    }

    SDL_DestroyTexture(pixelTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    if (renderedImages > 0)
    {
        std::cout << "Average render time: "
                  << totalRenderMilliseconds / renderedImages << " ms\n";
    }
    return exitCode;
}
