#pragma once

#include <functional>
#include <memory>
#include <string>

#include <geom3/base.hpp>
#include <geom3/matrix4.hpp>
#include <blunder/material.hpp>
#include <blunder/aabb.hpp>

class Object
{
public:
                            Object(std::string name, const Material& material, std::unique_ptr<Shape> shape);
    virtual                 ~Object() = default;

    const std::string&      name() const;
    const Material&         material() const;
    // const Color&            color() const;
    virtual Color           albedo_at(const Vector3& point) const;
    Vector3                 normal_at(const Vector3& point) const;
    std::vector<Hit>        intersects(const Line3& line) const;
    std::optional<Hit>      closest_intersection(const Line3& line) const;

    const Matrix4&          transform() const { return transform_; }
    void                    set_transform(const Matrix4& transform)
    {
        transform_ = transform;
        transformed_shape_dirty_ = true;
    }

    const Shape*                transformed_shape() const;
    virtual void                transform_shape() const = 0;

    const std::optional<AABB>&  bounding_box() const  {return bounding_box_;}
    
protected:
    std::string                             name_;
    std::reference_wrapper<const Material>  material_;
    std::unique_ptr<Shape>                  shape_;
    
    Matrix4                                 transform_ = Matrix4::identity();
    mutable std::unique_ptr<Shape>          transformed_shape_ = nullptr;
    mutable std::optional<AABB>             bounding_box_ = std::nullopt;
    mutable bool                            transformed_shape_dirty_ = true;
};

