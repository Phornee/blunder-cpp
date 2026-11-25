#include <blunder/primitives/triangle_object.hpp>

#include <utility>

#include <geom3/triangle3.hpp>
#include <algorithm>

TriangleObject::TriangleObject(
    std::string name,
    const Material& material,
    const Vector3& point_a,
    const Vector3& point_b,
    const Vector3& point_c,
    const Vector2& uv_a,
    const Vector2& uv_b,
    const Vector2& uv_c)
    : Object(
          std::move(name),
          material,
          std::make_unique<Triangle3>(point_a, point_b, point_c)),
      a_uv_(uv_a),
      b_uv_(uv_b),
      c_uv_(uv_c)
{
    transformed_shape_ = std::make_unique<Triangle3>(point_a, point_b, point_c);
}

void TriangleObject::transform_shape() const
{
    Triangle3* triangle = dynamic_cast<Triangle3*>(shape_.get());
    Vector3 transformed_a = transform_ * triangle->a;
    Vector3 transformed_b = transform_ * triangle->b;
    Vector3 transformed_c = transform_ * triangle->c;

    transformed_shape_ = std::make_unique<Triangle3>(transformed_a, transformed_b, transformed_c);
    const Vector3 minimum(
        std::min({transformed_a.x, transformed_b.x, transformed_c.x}),
        std::min({transformed_a.y, transformed_b.y, transformed_c.y}),
        std::min({transformed_a.z, transformed_b.z, transformed_c.z}));
    const Vector3 maximum(
        std::max({transformed_a.x, transformed_b.x, transformed_c.x}),
        std::max({transformed_a.y, transformed_b.y, transformed_c.y}),
        std::max({transformed_a.z, transformed_b.z, transformed_c.z}));
    bounding_box_ = AABB(minimum, maximum);
}


Color TriangleObject::albedo_at(const Vector3& point) const
{
    const Vector3 barycentric = shape_->barycentric(point);
    const Vector2 uv =
        a_uv_ * barycentric.x + b_uv_ * barycentric.y + c_uv_ * barycentric.z;
    return material_.get().albedo_at(uv);
}
