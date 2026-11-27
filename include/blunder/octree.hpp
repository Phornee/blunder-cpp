#pragma once

#include <array>
#include <cstddef>
#include <memory>
#include <vector>

#include <blunder/aabb.hpp>

class Line3;

class MeshOctree
{
public:
    explicit MeshOctree(const std::vector<AABB>& triangle_bounds);

    std::vector<std::size_t> ray_candidates(const Line3& ray) const;

private:
    MeshOctree(const AABB& bounds, std::vector<std::size_t> triangle_indices,
        const std::vector<AABB>& triangle_bounds,
        std::size_t depth);
    void subdivide(const std::vector<AABB>& triangle_bounds, std::size_t depth);
    void collect_ray_candidates(const Line3& ray, std::vector<std::size_t>& candidates) const;

    AABB                                        bounds_;
    std::vector<std::size_t>                    triangle_indices_;
    std::array<std::unique_ptr<MeshOctree>, 8>  children_{};
};
