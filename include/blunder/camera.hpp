#pragma once

#include <geom3/Vector3.hpp>
#include <geom3/vector2.hpp>

class Camera
{
public:
    Camera(const Vector3& position, const Vector3& view_direction, const Vector2& sensor_size, double focal_length);

    Vector3 sensor_center() const;
    const Vector3& sensor_u() const;
    const Vector3& sensor_v() const;
    const Vector3& view_direction() const;
    void set_view_direction(const Vector3& view_direction);

    Vector3 position;
    Vector2 sensor_size;
    double focal_length;

private:
    Vector3 view_direction_;
    Vector3 sensor_u_;
    Vector3 sensor_v_;
    Vector3 sensor_center_;

    void update_sensor();
};
