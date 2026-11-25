#pragma once

#include <optional>
#include <vector>

#include <geom3/Vector3.hpp>

class Line3;

class AABB
{
public:
    AABB(const Vector3& minimum, const Vector3& maximum);

    const Vector3& minimum() const noexcept;
    const Vector3& maximum() const noexcept;
    void set_bounds(const Vector3& minimum, const Vector3& maximum);
    void set_from_vertices(const std::vector<Vector3>& vertices);
    bool contains(const Vector3& point) const noexcept;
    bool intersects(const Line3& ray) const;
    std::optional<double> closest_intersection(const Line3& ray) const;

private:
    Vector3 minimum_;
    Vector3 maximum_;
};
