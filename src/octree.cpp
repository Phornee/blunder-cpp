#include <blunder/octree.hpp>

#include <algorithm>
#include <array>
#include <utility>

#include <geom3/base.hpp>
#include <geom3/line3.hpp>

namespace
{
    constexpr std::size_t MaxLeafTriangles = 8;
    constexpr std::size_t MaxDepth = 10;

    AABB combined_bounds(const std::vector<AABB>& bounds)
    {
        if (bounds.empty()) {
            return AABB(Vector3::zero, Vector3::zero);
        }

        Vector3 minimum = bounds.front().minimum();
        Vector3 maximum = bounds.front().maximum();
        for (const AABB& item : bounds) {
            minimum.x = std::min(minimum.x, item.minimum().x);
            minimum.y = std::min(minimum.y, item.minimum().y);
            minimum.z = std::min(minimum.z, item.minimum().z);
            maximum.x = std::max(maximum.x, item.maximum().x);
            maximum.y = std::max(maximum.y, item.maximum().y);
            maximum.z = std::max(maximum.z, item.maximum().z);
        }
        return AABB(minimum, maximum);
    }

    std::vector<std::size_t> make_triangle_indices(std::size_t count)
    {
        std::vector<std::size_t> indices;
        indices.reserve(count);
        for (std::size_t index = 0; index < count; ++index) {
            indices.push_back(index);
        }
        return indices;
    }

    /**
     * @brief Computes the bounds of one octant of an AABB.
     * @param bounds The AABB to divide into eight octants.
     * @param octant Octant index from 0 to 7; bits 0, 1, and 2 select the
     * upper half along x, y, and z, respectively.
     * @return The AABB for the selected octant.
     */
    AABB octant_bounds(const AABB& bounds, std::size_t octant)
    {
        const Vector3& minimum = bounds.minimum();
        const Vector3& maximum = bounds.maximum();
        const Vector3 midpoint = minimum + (maximum - minimum) * 0.5;

        const bool upper_x = (octant & 1U) != 0;
        const bool upper_y = (octant & 2U) != 0;
        const bool upper_z = (octant & 4U) != 0;
        return AABB(
            Vector3{
                upper_x ? midpoint.x : minimum.x,
                upper_y ? midpoint.y : minimum.y,
                upper_z ? midpoint.z : minimum.z},
            Vector3{
                upper_x ? maximum.x : midpoint.x,
                upper_y ? maximum.y : midpoint.y,
                upper_z ? maximum.z : midpoint.z});
    }

    bool contains_bounds(const AABB& outer, const AABB& inner)
    {
        return outer.minimum().x <= inner.minimum().x
            && outer.minimum().y <= inner.minimum().y
            && outer.minimum().z <= inner.minimum().z
            && outer.maximum().x >= inner.maximum().x
            && outer.maximum().y >= inner.maximum().y
            && outer.maximum().z >= inner.maximum().z;
    }
}

MeshOctree::MeshOctree(const std::vector<AABB>& triangle_bounds)
    : MeshOctree(
          combined_bounds(triangle_bounds),
          make_triangle_indices(triangle_bounds.size()),
          triangle_bounds,
          0)
{
}

MeshOctree::MeshOctree(
    const AABB& bounds,
    std::vector<std::size_t> triangle_indices,
    const std::vector<AABB>& triangle_bounds,
    std::size_t depth)
    : bounds_(bounds),
      triangle_indices_(std::move(triangle_indices))
{
    subdivide(triangle_bounds, depth);
}

void MeshOctree::subdivide(
    const std::vector<AABB>& triangle_bounds,
    std::size_t depth)
{
    if (triangle_indices_.size() <= MaxLeafTriangles || depth >= MaxDepth) {
        return;
    }

    std::vector<AABB> child_bounds;
    child_bounds.reserve(8);
    for (std::size_t octant = 0; octant < 8; ++octant) {
        child_bounds.push_back(octant_bounds(bounds_, octant));
    }

    std::array<std::vector<std::size_t>, 8> child_triangles;
    std::vector<std::size_t> remaining_triangles;
    for (const std::size_t triangle_index : triangle_indices_) {
        bool assigned_to_child = false;
        for (std::size_t octant = 0; octant < 8; ++octant) {
            if (contains_bounds(child_bounds[octant], triangle_bounds[triangle_index])) {
                child_triangles[octant].push_back(triangle_index);
                assigned_to_child = true;
                break;
            }
        }
        if (!assigned_to_child) {
            remaining_triangles.push_back(triangle_index);
        }
    }

    if (remaining_triangles.size() == triangle_indices_.size()) {
        return;
    }

    triangle_indices_ = std::move(remaining_triangles);
    for (std::size_t octant = 0; octant < 8; ++octant) {
        if (!child_triangles[octant].empty()) {
            children_[octant] = std::unique_ptr<MeshOctree>(new MeshOctree(
                child_bounds[octant],
                std::move(child_triangles[octant]),
                triangle_bounds,
                depth + 1));
        }
    }
}

std::vector<std::size_t> MeshOctree::ray_candidates(const Line3& ray) const
{
    std::vector<std::size_t> candidates;
    collect_ray_candidates(ray, candidates);
    std::sort(candidates.begin(), candidates.end());
    return candidates;
}

/**
 * @brief Appends triangle indices from octree nodes intersected by a ray.
 * @param ray Ray used to test each node's bounds.
 * @param candidates Output vector to append candidate triangle indices to.
 *
 * Stops traversing a node when its bounds do not intersect the ray;
 * otherwise, collects its indices and recursively visits its children.
 */
void MeshOctree::collect_ray_candidates(
    const Line3& ray,
    std::vector<std::size_t>& candidates) const
{
    if (!bounds_.intersects(ray)) {
        return;
    }

    candidates.insert(
        candidates.end(),
        triangle_indices_.begin(),
        triangle_indices_.end());
    for (const std::unique_ptr<MeshOctree>& child : children_) {
        if (child) {
            child->collect_ray_candidates(ray, candidates);
        }
    }
}

