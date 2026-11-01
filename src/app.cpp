#include <chrono>
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
bool DrawFpsHud(SDL_Renderer* renderer, int fps, const MiniFont& miniFont)
{
    constexpr int scale = 1;
    constexpr int characterAdvance = 6 * scale;
    constexpr int padding = 8 * scale;
    const std::string text = "FPS: " + std::to_string(fps);
    const int textWidth = static_cast<int>(text.size()) * characterAdvance - (characterAdvance - 10);
    const SDL_FRect background = {
        static_cast<float>(::Blunder::PixelWidth - textWidth - 2 * padding - 12),
        12.0F,
        static_cast<float>(textWidth + 2 * padding),
        7.0F * scale + 2 * padding};

    if (!SDL_SetRenderDrawColor(renderer, 10, 14, 20, 255) ||
        !SDL_RenderFillRect(renderer, &background))
    {
        return false;
    }

    return miniFont.DrawText(
        renderer,
        text,
        static_cast<int>(background.x) + padding,
        static_cast<int>(background.y) + padding,
        scale,
        {255, 255, 255, 255});
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
    int fps)
{
    const Blunder::PixelGrid& pixels = blunder.GetPixelGrid();

    return SDL_UpdateTexture(
            pixelTexture,
            nullptr,
            pixels.data(),
            static_cast<int>(sizeof(pixels[0]))) &&
        SDL_RenderClear(renderer) &&
        SDL_RenderTexture(renderer, pixelTexture, nullptr, nullptr) &&
        DrawFpsHud(renderer, fps, miniFont) &&
        SDL_RenderPresent(renderer);
}
}

/// Runs the geometry demo and SDL application until the window is closed or Escape is pressed.
/// Initializes SDL resources, processes events, updates the FPS estimate, and cleans up.
/// @return 0 on normal exit, or 1 if SDL initialization or rendering fails.
int main()
{
    ::Blunder blunder;
    MiniFont miniFont;

    std::cout << "Close the Blunder window or press Escape to exit.\n";

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
        }

        if (!running)
        {
            break;
        }

        if (!RenderFrame(renderer, pixelTexture, blunder, miniFont, displayedFps))
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
    return exitCode;
}
