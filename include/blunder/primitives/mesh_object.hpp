#pragma once

#include <string>

#include <blunder/primitives/object.hpp>

class MeshObject final : public Object
{
public:
    MeshObject(
        std::string name,
        const Material& material,
        const std::string& file);

    const std::string& filename() const;
    Color albedo_at(const Vector3& point) const override;
    virtual void            transform_shape() const override;

private:
    std::string file_;
};
