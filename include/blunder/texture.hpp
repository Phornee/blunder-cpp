#pragma once

#include <string>
#include <vector>

#include <blunder/color.hpp>
#include <geom3/vector2.hpp>

class Texture
{
public:
    explicit Texture(const std::string& image_path);

    Color sample(const Vector2& coordinates) const;

private:
    int width_;
    int height_;
    std::vector<Color> pixels_;
};
