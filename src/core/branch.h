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
        , light_exposure(1.0f)
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

    // Phase 2: Tropism support - curved paths
    std::vector<Vec3> curve_points;  // Points along curved path (if tropism applied)
    float light_exposure;             // Cached light exposure value (0-1)

    // Helper methods
    Vec3 end_pos() const {
        // If we have curved points, use the last one
        if (!curve_points.empty()) {
            return curve_points.back();
        }
        // Otherwise use straight line calculation
        return start_pos + (direction * length);
    }

    void add_child(std::shared_ptr<Branch> child) {
        child->parent = this;
        child->depth = this->depth + 1;
        children.push_back(child);
    }

    // Get path points for rendering
    std::vector<Vec3> get_path_points(int segments = 1) const {
        // If we have pre-computed curved points, return those
        if (!curve_points.empty()) {
            return curve_points;
        }

        // Otherwise generate straight line
        std::vector<Vec3> points;
        for (int i = 0; i <= segments; ++i) {
            float t = static_cast<float>(i) / segments;
            points.push_back(start_pos + (direction * (length * t)));
        }
        return points;
    }

    // Phase 2: Generate curved path with tropism
    void generate_curved_path(int num_segments) {
        curve_points.clear();
        curve_points.push_back(start_pos);

        Vec3 current_pos = start_pos;
        Vec3 current_dir = direction;
        float segment_length = length / num_segments;

        for (int i = 1; i <= num_segments; ++i) {
            current_pos = current_pos + (current_dir * segment_length);
            curve_points.push_back(current_pos);
            // Direction gets updated by tropism system
        }
    }
};

} // namespace plantgrow
