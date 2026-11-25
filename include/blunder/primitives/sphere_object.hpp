#pragma once

#include <blunder/primitives/object.hpp>

class SphereObject final : public Object
{
public:
    SphereObject(
        std::string name,
        const Material& material,
        const Vector3& center,
        double radius);
    Color albedo_at(const Vector3& point) const override;
    virtual void            transform_shape() const override;
};
