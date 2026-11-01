#include <blunder/light.hpp>

#include <utility>

Light::Light(Vector3 color_value, std::string name_value)
    : color_(color_value), name_(std::move(name_value))
{
}

bool Light::is_specular() const
{
    return false;
}

const Vector3& Light::color() const
{
    return color_;
}

const std::string& Light::name() const
{
    return name_;
}

DirectionalLight::DirectionalLight(Vector3 color, std::string name, Vector3 direction)
    : Light(color, std::move(name)), direction_(direction.normalized())
{
}

double DirectionalLight::intensity(const Vector3&, const Vector3& normal) const
{
    return normal.dot(direction_) < 0.0 ? 0.0 : normal.dot(direction_);
}

bool DirectionalLight::can_be_eclipsed() const
{
    return true;
}

Vector3 DirectionalLight::direction_to_light(const Vector3&) const
{
    return direction_ * 1000000.0;
}

bool DirectionalLight::is_specular() const
{
    return true;
}

PointLight::PointLight(Vector3 color, std::string name, Vector3 position)
    : Light(color, std::move(name)), position_(position)
{
}

double PointLight::intensity(const Vector3& hit_point, const Vector3& normal) const
{
    const Vector3 direction = direction_to_light(hit_point).normalized();
    const double cosine = normal.dot(direction);
    return cosine < 0.0 ? 0.0 : cosine;
}

bool PointLight::can_be_eclipsed() const
{
    return true;
}

Vector3 PointLight::direction_to_light(const Vector3& hit_point) const
{
    return position_ - hit_point;
}

bool PointLight::is_specular() const
{
    return true;
}

AmbientLight::AmbientLight(Vector3 color, std::string name)
    : Light(color, std::move(name))
{
}

double AmbientLight::intensity(const Vector3&, const Vector3&) const
{
    return 1.0;
}

bool AmbientLight::can_be_eclipsed() const
{
    return false;
}

Vector3 AmbientLight::direction_to_light(const Vector3&) const
{
    return Vector3::zero;
}
