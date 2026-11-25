#include <blunder/aabb.hpp>

#include <algorithm>
#include <limits>
#include <stdexcept>

#include <geom3/line3.hpp>

namespace
{
    void validate_bounds(const Vector3& minimum, const Vector3& maximum)
    {
        if (minimum.x > maximum.x ||
            minimum.y > maximum.y ||
            minimum.z > maximum.z)
        {
            throw std::invalid_argument("AABB minimum must not exceed its maximum");
        }
    }
}

AABB::AABB(const Vector3& minimum, const Vector3& maximum)
    : minimum_(minimum),
      maximum_(maximum)
{
    validate_bounds(minimum_, maximum_);
}

const Vector3& AABB::minimum() const noexcept
{
    return minimum_;
}

const Vector3& AABB::maximum() const noexcept
{
    return maximum_;
}

void AABB::set_bounds(const Vector3& minimum, const Vector3& maximum)
{
    validate_bounds(minimum, maximum);
    minimum_ = minimum;
    maximum_ = maximum;
}

void AABB::set_from_vertices(const std::vector<Vector3>& vertices)
{
    if (vertices.empty()) {
        throw std::invalid_argument("Cannot calculate an AABB from an empty vertex list");
    }

    Vector3 minimum = vertices.front();
    Vector3 maximum = vertices.front();
    for (std::size_t i = 1; i < vertices.size(); ++i) {
        const Vector3& vertex = vertices[i];
        minimum.x = std::min(minimum.x, vertex.x);
        minimum.y = std::min(minimum.y, vertex.y);
        minimum.z = std::min(minimum.z, vertex.z);
        maximum.x = std::max(maximum.x, vertex.x);
        maximum.y = std::max(maximum.y, vertex.y);
        maximum.z = std::max(maximum.z, vertex.z);
    }

    set_bounds(minimum, maximum);
}

bool AABB::contains(const Vector3& point) const noexcept
{
    return point.x >= minimum_.x && point.x <= maximum_.x &&
           point.y >= minimum_.y && point.y <= maximum_.y &&
           point.z >= minimum_.z && point.z <= maximum_.z;
}

bool AABB::intersects(const Line3& ray) const
{
    return closest_intersection(ray).has_value();
}

std::optional<double> AABB::closest_intersection(const Line3& ray) const
{
    double t_near = -std::numeric_limits<double>::infinity();
    double t_far = std::numeric_limits<double>::infinity();

    const auto intersect_axis = [&t_near, &t_far](
                                    double origin,
                                    double direction,
                                    double minimum,
                                    double maximum) {
        if (direction == 0.0) {
            return origin >= minimum && origin <= maximum;
        }

        const double inverse_direction = 1.0 / direction;
        double t1 = (minimum - origin) * inverse_direction;
        double t2 = (maximum - origin) * inverse_direction;
        if (t1 > t2) {
            std::swap(t1, t2);
        }
        t_near = std::max(t_near, t1);
        t_far = std::min(t_far, t2);
        return t_near <= t_far;
    };

    if (!intersect_axis(ray.a.x, ray.v.x, minimum_.x, maximum_.x) ||
        !intersect_axis(ray.a.y, ray.v.y, minimum_.y, maximum_.y) ||
        !intersect_axis(ray.a.z, ray.v.z, minimum_.z, maximum_.z) ||
        t_far < 0.0)
    {
        return std::nullopt;
    }

    return t_near >= 0.0 ? t_near : t_far;
}