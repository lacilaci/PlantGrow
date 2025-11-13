#pragma once

#include "types.h"
#include <vector>
#include <memory>

namespace plantgrow {

// Represents a single branch segment in the tree
class Branch {
public:
    Branch(const Vec3& start, const Vec3& direction, float length, float radius)
        : start_pos(start)
        , direction(direction.normalized())
        , length(length)
        , radius(radius)
        , age(0)
        , depth(0)
        , parent(nullptr)
    {}

    // Position and geometry
    Vec3 start_pos;
    Vec3 direction;  // Normalized direction vector
    float length;
    float radius;

    // Hierarchy
    int depth;  // Distance from trunk (0 = trunk)
    Branch* parent;
    std::vector<std::shared_ptr<Branch>> children;

    // Growth properties
    int age;  // Age in years or iterations

    // Helper methods
    Vec3 end_pos() const {
        return start_pos + (direction * length);
    }

    void add_child(std::shared_ptr<Branch> child) {
        child->parent = this;
        child->depth = this->depth + 1;
        children.push_back(child);
    }

    // For Phase 1: Simple path generation
    std::vector<Vec3> get_path_points(int segments = 1) const {
        std::vector<Vec3> points;
        for (int i = 0; i <= segments; ++i) {
            float t = static_cast<float>(i) / segments;
            points.push_back(start_pos + (direction * (length * t)));
        }
        return points;
    }
};

} // namespace plantgrow
