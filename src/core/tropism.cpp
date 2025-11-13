#include "tropism.h"
#include <algorithm>
#include <cmath>

namespace plantgrow {

TropismSystem::TropismSystem(const TropismParams& params, const Environment& env)
    : params_(params)
    , environment_(env)
{}

Vec3 TropismSystem::apply_tropism(const Vec3& current_direction,
                                  const Vec3& position,
                                  int branch_depth,
                                  int branch_age) const {
    Vec3 result = current_direction.normalized();

    // Apply phototropism if enabled
    if (params_.phototropism_enabled) {
        result = apply_phototropism(result, position);
    }

    // Apply gravitropism if enabled
    if (params_.gravitropism_enabled) {
        result = apply_gravitropism(result, branch_depth);
    }

    return result.normalized();
}

Vec3 TropismSystem::apply_phototropism(const Vec3& current_direction,
                                       const Vec3& position) const {
    // Calculate direction toward light source
    Vec3 to_light = (environment_.light_position - position).normalized();

    // Calculate distance to light (for falloff)
    float distance = (environment_.light_position - position).length();
    float distance_falloff = 1.0f;
    if (params_.response_distance > 0) {
        distance_falloff = std::max(0.0f, 1.0f - (distance / (params_.response_distance * 100.0f)));
    }

    // Calculate how much the current direction is already aligned with light
    float alignment = current_direction.dot(to_light);
    // If already well-aligned, reduce bending (prevents overcorrection)
    float alignment_factor = 1.0f - std::max(0.0f, alignment) * 0.5f;

    // Effective strength with distance falloff
    float effective_strength = params_.phototropism_strength * distance_falloff * alignment_factor;

    // Bend toward light
    return bend_toward_vector(current_direction, to_light, effective_strength);
}

Vec3 TropismSystem::apply_gravitropism(const Vec3& current_direction,
                                       int branch_depth) const {
    // Main trunk (depth 0) has strong apical dominance - resists gravity
    // Side branches droop more
    float depth_factor = 1.0f - (params_.apical_dominance / (branch_depth + 1.0f));
    depth_factor = std::clamp(depth_factor, 0.0f, 1.0f);

    // Effective gravitropism strength based on branch depth
    float effective_strength = params_.gravitropism_strength * depth_factor;

    // For main trunk, gravity provides upward correction
    // For side branches, gravity causes droop
    Vec3 gravity_response;
    if (branch_depth == 0) {
        // Main trunk: resist gravity, grow upward
        gravity_response = Vec3(0, 1, 0); // Upward
        effective_strength *= 0.5f; // Weaker effect on trunk
    } else {
        // Side branches: affected by gravity (droop)
        gravity_response = environment_.gravity_direction;
    }

    return bend_toward_vector(current_direction, gravity_response, effective_strength);
}

float TropismSystem::compute_light_exposure(const Vec3& position,
                                            const Vec3& direction) const {
    // Calculate direction to light
    Vec3 to_light = (environment_.light_position - position).normalized();

    // Dot product gives alignment (-1 to 1)
    float alignment = direction.dot(to_light);

    // Convert to 0-1 range (0 = facing away, 1 = facing light)
    float exposure = (alignment + 1.0f) * 0.5f;

    // Add ambient light
    exposure = std::max(exposure, environment_.ambient_light);

    return std::clamp(exposure, 0.0f, 1.0f);
}

Vec3 TropismSystem::bend_toward_vector(const Vec3& current,
                                       const Vec3& target,
                                       float strength) const {
    // Clamp strength to valid range
    strength = std::clamp(strength, 0.0f, 1.0f);

    // Linear interpolation between current and target
    // strength = 0: no change
    // strength = 1: fully toward target
    Vec3 result = current * (1.0f - strength) + target * strength;

    return result.normalized();
}

float TropismSystem::get_age_modifier(int age) const {
    // Young branches are more flexible (higher modifier)
    // Old branches are more rigid (lower modifier)
    float modifier = 1.0f - (age * params_.age_sensitivity * 0.01f);
    return std::clamp(modifier, 0.3f, 1.0f);
}

} // namespace plantgrow
