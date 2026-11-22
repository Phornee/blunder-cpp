#pragma once

class Color
{
public:
    constexpr Color(double red = 0.0, double green = 0.0, double blue = 0.0)
        : red_(red), green_(green), blue_(blue)
    {
    }

    constexpr double r() const { return red_; }
    constexpr double g() const { return green_; }
    constexpr double b() const { return blue_; }

    constexpr void set_r(double red) { red_ = red; }
    constexpr void set_g(double green) { green_ = green; }
    constexpr void set_b(double blue) { blue_ = blue; }

    constexpr Color operator+(const Color& other) const
    {
        return {red_ + other.red_, green_ + other.green_, blue_ + other.blue_};
    }

    constexpr Color operator*(const Color& other) const
    {
        return {red_ * other.red_, green_ * other.green_, blue_ * other.blue_};
    }

    constexpr Color operator*(double scalar) const
    {
        return {red_ * scalar, green_ * scalar, blue_ * scalar};
    }

    friend constexpr Color operator*(double scalar, const Color& color)
    {
        return color * scalar;
    }

    constexpr Color operator/(double scalar) const
    {
        return {red_ / scalar, green_ / scalar, blue_ / scalar};
    }

private:
    double red_;
    double green_;
    double blue_;
};
