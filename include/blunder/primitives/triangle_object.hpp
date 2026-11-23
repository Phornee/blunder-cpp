#pragma once

#include <blunder/primitives/object.hpp>

class TriangleObject final : public Object
{
public:
    TriangleObject(
        std::string name,
        const Material& material,
        const Vector3& point_a,
        const Vector3& point_b,
        const Vector3& point_c,
        const Vector2& uv_a = Vector2{0.0, 0.0},
        const Vector2& uv_b = Vector2{1.0, 0.0},
        const Vector2& uv_c = Vector2{0.5, 1.0}
    );
    Color color_at(const Vector3& point) const override;
    virtual void            transform_shape() const override;
private:
    Vector2 a_uv_;
    Vector2 b_uv_;
    Vector2 c_uv_;
};
