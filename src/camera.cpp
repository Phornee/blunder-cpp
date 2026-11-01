#include <blunder/camera.hpp>

Camera::Camera(
    const Vector3& position_value,
    const Vector3& view_direction,
    const Vector2& sensor_size_value,
    double focal_length_value)
    : position(position_value),
      sensor_size(sensor_size_value),
      focal_length(focal_length_value),
      view_direction_(view_direction.normalized())
{
    update_sensor();
}

Vector3 Camera::sensor_center() const
{
    return sensor_center_;
}

const Vector3& Camera::sensor_u() const
{
    return sensor_u_;
}

const Vector3& Camera::sensor_v() const
{
    return sensor_v_;
}

const Vector3& Camera::view_direction() const
{
    return view_direction_;
}

void Camera::set_view_direction(const Vector3& view_direction)
{
    view_direction_ = view_direction.normalized();
    update_sensor();
}

void Camera::update_sensor()
{
    sensor_center_ = position + view_direction_ * focal_length;
    sensor_u_ = view_direction_.cross(Vector3::up).normalized();
    sensor_v_ = sensor_u_.cross(view_direction_).normalized();
}
