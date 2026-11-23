#pragma once

#include <functional>
#include <memory>
#include <string>

#include <geom3/base.hpp>
#include <geom3/matrix4.hpp>
#include <blunder/material.hpp>

class Object
{
public:
                            Object(std::string name, const Material& material, std::unique_ptr<Shape> shape);
    virtual                 ~Object() = default;

    const std::string&      name() const;
    const Material&         material() const;
    const Color&            color() const;
    virtual Color           color_at(const Vector3& point) const;
    Vector3                 normal(const Vector3& point) const;
    std::optional<double>   closest_intersection(const Line3& line) const;
    void                    apply_transform(const Matrix4& transform);
    const Shape*            transformed_shape() const;
    virtual void            transform_shape() const = 0;
    
protected:
    std::string                             name_;
    std::reference_wrapper<const Material>  material_;
    std::unique_ptr<Shape>                  shape_;
    
    int                                     frame_ = 0;
    Matrix4                                 transform_ = Matrix4::identity();
    mutable std::unique_ptr<Shape>          transformed_shape_ = nullptr;
};

