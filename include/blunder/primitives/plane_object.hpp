#pragma once

#include <blunder/primitives/object.hpp>

class PlaneObject final : public Object
{
public:
    PlaneObject(
        std::string name,
        const Material& material,
        const Vector3& point,
        const Vector3& normal);
    Color albedo_at(const Vector3& point) const override;
    virtual void            transform_shape() const override;
};
