#pragma once

#include "types.h"
#include "branch.h"
#include <vector>
#include <memory>

namespace plantgrow {

// Resource system parameters
struct ResourceParams {
    // Light capture
    bool light_competition_enabled = true;
    float base_light_level = 1.0f;           // Base ambient light
    float occlusion_radius = 2.0f;           // Radius for checking branch occlusion
    float occlusion_falloff = 0.5f;          // How fast occlusion effect falls off

    // Resource allocation
    float photosynthesis_efficiency = 1.0f;  // How efficiently light converts to resources
    float resource_transport_rate = 0.8f;    // How efficiently resources flow to parent
    float maintenance_cost = 0.1f;           // Resource cost per unit branch length

    // Pruning thresholds
    bool pruning_enabled = true;
    float min_light_threshold = 0.15f;       // Minimum light to survive
    float min_resource_threshold = 0.2f;     // Minimum resource balance to survive
    int pruning_grace_period = 2;            // Age before pruning can occur

    // Branch competition
    float competition_radius = 1.5f;         // Radius for branch competition
    float dominance_factor = 0.7f;           // How much higher branches dominate
};

// Resource tracking for a branch
struct ResourceState {
    float light_capture = 1.0f;      // 0-1: amount of light this branch receives
    float resource_balance = 1.0f;   // Net resource (production - cost)
    float accumulated_deficit = 0.0f; // Tracks prolonged resource deficit
    bool marked_for_pruning = false;
    int deficit_duration = 0;        // How many cycles in deficit
};

// Resource allocation system
class ResourceSystem {
public:
    ResourceSystem(const ResourceParams& params);
    ~ResourceSystem() = default;

    // Main simulation step
    void calculate_resources(std::vector<std::shared_ptr<Branch>>& branches);

    // Get resource state for a branch
    ResourceState get_state(int branch_id) const;

    // Prune branches that fail resource threshold
    std::vector<int> identify_pruned_branches() const;

    // Reset simulation (call before new tree generation)
    void reset();

private:
    ResourceParams params_;
    std::vector<ResourceState> branch_states_;

    // Light capture simulation
    void calculate_light_capture(std::vector<std::shared_ptr<Branch>>& branches);
    float calculate_occlusion(const Branch& branch,
                             const std::vector<std::shared_ptr<Branch>>& branches) const;

    // Resource allocation
    void calculate_resource_flow(std::vector<std::shared_ptr<Branch>>& branches);
    float calculate_photosynthesis(const Branch& branch, float light_capture) const;
    float calculate_maintenance_cost(const Branch& branch) const;

    // Branch competition
    void apply_competition(std::vector<std::shared_ptr<Branch>>& branches);
    float calculate_competition_factor(const Branch& branch,
                                       const std::vector<std::shared_ptr<Branch>>& branches) const;

    // Pruning decisions
    void evaluate_pruning(std::vector<std::shared_ptr<Branch>>& branches);
    bool should_prune(const Branch& branch, const ResourceState& state) const;
};

} // namespace plantgrow
