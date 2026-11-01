#pragma once

#include <string>

#include <geom3/Vector3.hpp>

class Light
{
public:
    Light(Vector3 color, std::string name);
    virtual ~Light() = default;

    virtual double intensity(const Vector3& hit_point, const Vector3& normal) const = 0;
    virtual bool can_be_eclipsed() const = 0;
    virtual Vector3 direction_to_light(const Vector3& hit_point) const = 0;
    virtual bool is_specular() const;

    const Vector3& color() const;
    const std::string& name() const;

private:
    Vector3 color_;
    std::string name_;
};

class DirectionalLight final : public Light
{
public:
    DirectionalLight(Vector3 color, std::string name, Vector3 direction);

    double intensity(const Vector3& hit_point, const Vector3& normal) const override;
    bool can_be_eclipsed() const override;
    Vector3 direction_to_light(const Vector3& hit_point) const override;
    bool is_specular() const override;

private:
    Vector3 direction_;
};

class PointLight final : public Light
{
public:
    PointLight(Vector3 color, std::string name, Vector3 position);

    double intensity(const Vector3& hit_point, const Vector3& normal) const override;
    bool can_be_eclipsed() const override;
    Vector3 direction_to_light(const Vector3& hit_point) const override;
    bool is_specular() const override;

private:
    Vector3 position_;
};

class AmbientLight final : public Light
{
public:
    AmbientLight(Vector3 color, std::string name);

    double intensity(const Vector3& hit_point, const Vector3& normal) const override;
    bool can_be_eclipsed() const override;
    Vector3 direction_to_light(const Vector3& hit_point) const override;
};
