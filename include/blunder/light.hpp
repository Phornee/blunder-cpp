#pragma once

#include <string>

#include <blunder/color.hpp>
#include <geom3/Vector3.hpp>

class Light
{
public:
                        Light(std::string name, Color color);
    virtual             ~Light() = default;

    virtual double      intensity(const Vector3& hit_point, const Vector3& normal) const = 0;
    virtual bool        can_be_eclipsed() const = 0;
    virtual Vector3     direction_to_light(const Vector3& hit_point) const = 0;
    virtual bool        is_specular() const;

    const std::string&  name() const;
    const Color&        color() const;

private:
    std::string         name_;
    Color               color_;
};

class DirectionalLight final : public Light
{
public:
                DirectionalLight(std::string name, Color color, Vector3 direction);

    double      intensity(const Vector3& hit_point, const Vector3& normal) const override;
    bool        can_be_eclipsed() const override;
    Vector3     direction_to_light(const Vector3& hit_point) const override;
    bool        is_specular() const override;

private:
    Vector3 direction_;
};

class PointLight final : public Light
{
public:
                PointLight(std::string name, Color color, Vector3 position);

    double      intensity(const Vector3& hit_point, const Vector3& normal) const override;
    bool        can_be_eclipsed() const override;
    Vector3     direction_to_light(const Vector3& hit_point) const override;
    bool        is_specular() const override;

private:
    Vector3 position_;
};

class AmbientLight final : public Light
{
public:
                AmbientLight(std::string name, Color color);

    double      intensity(const Vector3& hit_point, const Vector3& normal) const override;
    bool        can_be_eclipsed() const override;
    Vector3     direction_to_light(const Vector3& hit_point) const override;
};
