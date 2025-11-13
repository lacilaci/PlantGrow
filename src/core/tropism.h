#pragma once

#include "types.h"
#include "branch.h"
#include <vector>

namespace plantgrow {

// Environment parameters for tropism simulation
struct Environment {
    Vec3 light_direction;      // Direction to primary light source (normalized)
    Vec3 gravity_direction;    // Gravity direction (typically Vec3(0, -1, 0))
    Vec3 light_position;       // Position of light source (for distance calculations)
    float ambient_light;       // Ambient light level (0-1)

    Environment()
        : light_direction(0, 1, 0)
        , gravity_direction(0, -1, 0)
        , light_position(0, 100, 0)
        , ambient_light(0.2f)
    {}
};

// Tropism configuration parameters
struct TropismParams {
    // Phototropism (light response)
    float phototropism_strength;     // 0-1, how strongly branches bend toward light
    float response_distance;          // Distance over which tropism is applied
    bool phototropism_enabled;

    // Gravitropism (gravity response)
    float gravitropism_strength;     // 0-1, how strongly branches respond to gravity
    bool gravitropism_enabled;

    // Advanced parameters
    float age_sensitivity;            // How much tropism strength varies with branch age
    float apical_dominance;          // How much main stem resists bending (0-1)

    TropismParams()
        : phototropism_strength(0.8f)
        , response_distance(5.0f)
        , phototropism_enabled(true)
        , gravitropism_strength(0.6f)
        , gravitropism_enabled(true)
        , age_sensitivity(0.5f)
        , apical_dominance(0.65f)
    {}
};

// Tropism system - applies environmental forces to growing branches
class TropismSystem {
public:
    TropismSystem(const TropismParams& params, const Environment& env);

    // Apply tropism to a branch's growth direction
    // Returns modified direction vector (normalized)
    Vec3 apply_tropism(const Vec3& current_direction,
                       const Vec3& position,
                       int branch_depth,
                       int branch_age) const;

    // Apply phototropism - bend toward light
    Vec3 apply_phototropism(const Vec3& current_direction,
                           const Vec3& position) const;

    // Apply gravitropism - respond to gravity
    Vec3 apply_gravitropism(const Vec3& current_direction,
                           int branch_depth) const;

    // Calculate light exposure at a position
    float compute_light_exposure(const Vec3& position,
                                const Vec3& direction) const;

    // Update environment (e.g., change light direction over time)
    void set_environment(const Environment& env) { environment_ = env; }
    void set_params(const TropismParams& params) { params_ = params; }

private:
    TropismParams params_;
    Environment environment_;

    // Helper: Bend direction toward target vector
    Vec3 bend_toward_vector(const Vec3& current,
                           const Vec3& target,
                           float strength) const;

    // Helper: Calculate age-based strength modifier
    float get_age_modifier(int age) const;
};

} // namespace plantgrow
