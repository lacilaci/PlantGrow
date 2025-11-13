#include "resources.h"
#include <cmath>
#include <algorithm>
#include <iostream>

namespace plantgrow {

ResourceSystem::ResourceSystem(const ResourceParams& params)
    : params_(params) {
}

void ResourceSystem::reset() {
    branch_states_.clear();
}

void ResourceSystem::calculate_resources(std::vector<std::shared_ptr<Branch>>& branches) {
    if (branches.empty()) {
        return;
    }

    // Initialize state for all branches
    if (branch_states_.size() != branches.size()) {
        branch_states_.resize(branches.size());
        for (auto& state : branch_states_) {
            state = ResourceState{};
        }
    }

    // Step 1: Calculate light capture (occlusion from other branches)
    if (params_.light_competition_enabled) {
        calculate_light_capture(branches);
    }

    // Step 2: Apply branch competition
    apply_competition(branches);

    // Step 3: Calculate resource flow (photosynthesis - costs)
    calculate_resource_flow(branches);

    // Step 4: Evaluate pruning decisions
    if (params_.pruning_enabled) {
        evaluate_pruning(branches);
    }

    // Update branch light exposure for visualization
    for (size_t i = 0; i < branches.size(); ++i) {
        branches[i]->light_exposure = branch_states_[i].light_capture;
    }
}

void ResourceSystem::calculate_light_capture(std::vector<std::shared_ptr<Branch>>& branches) {
    // Optimize for large trees: sample only a subset for occlusion checks
    const size_t MAX_BRANCHES_FOR_FULL_CHECK = 1000;
    bool use_sampling = branches.size() > MAX_BRANCHES_FOR_FULL_CHECK;

    for (size_t i = 0; i < branches.size(); ++i) {
        float occlusion = 0.0f;

        if (use_sampling) {
            // For large trees, use simplified calculation based on depth and height
            Vec3 pos = branches[i]->end_pos();
            float depth_factor = std::min(10, branches[i]->depth) / 10.0f;
            float height_factor = std::clamp((pos.y + 10.0f) / 20.0f, 0.0f, 1.0f);

            // Deeper branches get more occluded, higher branches get less
            occlusion = depth_factor * (1.0f - height_factor * 0.5f) * 0.5f;
        } else {
            occlusion = calculate_occlusion(*branches[i], branches);
        }

        // Light capture based on:
        // 1. Base ambient light
        // 2. Occlusion from other branches
        // 3. Branch orientation (upward facing branches get more light)
        Vec3 up(0, 1, 0);
        float orientation_bonus = std::max(0.0f, branches[i]->direction.dot(up)) * 0.3f;

        branch_states_[i].light_capture = params_.base_light_level *
                                          (1.0f - occlusion) +
                                          orientation_bonus;
        branch_states_[i].light_capture = std::clamp(branch_states_[i].light_capture, 0.0f, 1.0f);
    }
}

float ResourceSystem::calculate_occlusion(const Branch& branch,
                                         const std::vector<std::shared_ptr<Branch>>& branches) const {
    Vec3 branch_pos = branch.end_pos();
    float total_occlusion = 0.0f;
    int occluder_count = 0;

    for (const auto& other : branches) {
        if (other.get() == &branch) continue;

        // Only branches above can occlude
        Vec3 other_pos = other->end_pos();
        if (other_pos.y <= branch_pos.y) continue;

        // Check if within occlusion radius
        Vec3 diff = other_pos - branch_pos;
        float distance = diff.length();

        if (distance < params_.occlusion_radius) {
            // More occlusion for branches directly above
            float height_diff = other_pos.y - branch_pos.y;
            float horizontal_dist = std::sqrt(diff.x * diff.x + diff.z * diff.z);

            // Branches directly overhead occlude more
            float occlusion_factor = height_diff / (horizontal_dist + 0.1f);
            occlusion_factor = std::clamp(occlusion_factor, 0.0f, 1.0f);

            // Apply distance falloff
            float falloff = 1.0f - (distance / params_.occlusion_radius);
            total_occlusion += occlusion_factor * falloff * params_.occlusion_falloff;
            occluder_count++;
        }
    }

    // Normalize occlusion
    if (occluder_count > 0) {
        total_occlusion = std::min(0.9f, total_occlusion / std::sqrt(static_cast<float>(occluder_count)));
    }

    return total_occlusion;
}

void ResourceSystem::apply_competition(std::vector<std::shared_ptr<Branch>>& branches) {
    // Optimize for large trees: skip competition for performance
    const size_t MAX_BRANCHES_FOR_COMPETITION = 1000;
    if (branches.size() > MAX_BRANCHES_FOR_COMPETITION) {
        // Skip detailed competition for large trees (already factored in occlusion)
        return;
    }

    for (size_t i = 0; i < branches.size(); ++i) {
        float competition = calculate_competition_factor(*branches[i], branches);

        // Competition reduces effective light capture
        branch_states_[i].light_capture *= (1.0f - competition * 0.5f);
    }
}

float ResourceSystem::calculate_competition_factor(const Branch& branch,
                                                   const std::vector<std::shared_ptr<Branch>>& branches) const {
    Vec3 branch_pos = branch.end_pos();
    float competition = 0.0f;
    int competitor_count = 0;

    for (const auto& other : branches) {
        if (other.get() == &branch) continue;

        Vec3 other_pos = other->end_pos();
        Vec3 diff = other_pos - branch_pos;
        float distance = diff.length();

        if (distance < params_.competition_radius) {
            // Higher branches dominate lower ones
            float height_advantage = (other_pos.y - branch_pos.y) * params_.dominance_factor;

            // Same-level branches compete equally
            float competition_strength = 1.0f - (distance / params_.competition_radius);

            if (height_advantage > 0) {
                competition_strength *= (1.0f + height_advantage);
            }

            competition += competition_strength;
            competitor_count++;
        }
    }

    if (competitor_count > 0) {
        competition = std::min(0.8f, competition / static_cast<float>(competitor_count));
    }

    return competition;
}

void ResourceSystem::calculate_resource_flow(std::vector<std::shared_ptr<Branch>>& branches) {
    for (size_t i = 0; i < branches.size(); ++i) {
        // Photosynthesis (resource production from light)
        float production = calculate_photosynthesis(*branches[i], branch_states_[i].light_capture);

        // Maintenance cost (resource consumption)
        float cost = calculate_maintenance_cost(*branches[i]);

        // Net resource balance
        branch_states_[i].resource_balance = production - cost;

        // Track resource deficit over time
        if (branch_states_[i].resource_balance < 0) {
            branch_states_[i].accumulated_deficit += -branch_states_[i].resource_balance;
            branch_states_[i].deficit_duration++;
        } else {
            // Recovery: slowly reduce deficit
            branch_states_[i].accumulated_deficit *= 0.8f;
            if (branch_states_[i].accumulated_deficit < 0.01f) {
                branch_states_[i].deficit_duration = 0;
            }
        }
    }
}

float ResourceSystem::calculate_photosynthesis(const Branch& branch, float light_capture) const {
    // Photosynthesis scales with:
    // 1. Light capture (primary factor)
    // 2. Branch surface area (length * radius)
    // 3. Efficiency parameter

    float surface_area = branch.length * branch.radius * 2.0f;
    float production = light_capture * surface_area * params_.photosynthesis_efficiency;

    return production;
}

float ResourceSystem::calculate_maintenance_cost(const Branch& branch) const {
    // Maintenance cost scales with branch volume
    float volume = branch.length * branch.radius * branch.radius * 3.14159f;
    float cost = volume * params_.maintenance_cost;

    // Older branches have slightly higher maintenance costs
    float age_factor = 1.0f + (branch.age * 0.05f);

    return cost * age_factor;
}

void ResourceSystem::evaluate_pruning(std::vector<std::shared_ptr<Branch>>& branches) {
    int candidates = 0;
    float min_light = 1.0f;
    float max_light = 0.0f;
    float avg_light = 0.0f;

    for (size_t i = 0; i < branches.size(); ++i) {
        float light = branch_states_[i].light_capture;
        min_light = std::min(min_light, light);
        max_light = std::max(max_light, light);
        avg_light += light;

        if (should_prune(*branches[i], branch_states_[i])) {
            branch_states_[i].marked_for_pruning = true;
            candidates++;
        }
    }

    if (!branches.empty()) {
        avg_light /= branches.size();
        std::cout << "  Light stats - Min: " << min_light << ", Max: " << max_light << ", Avg: " << avg_light << std::endl;
        if (candidates > 0) {
            std::cout << "  Pruning candidates: " << candidates << std::endl;
        }
    }
}

bool ResourceSystem::should_prune(const Branch& branch, const ResourceState& state) const {
    // Don't prune main trunk (depth 0-1)
    if (branch.depth <= 1) {
        return false;
    }

    // Use depth as proxy for age - only prune branches beyond grace depth
    // (Since L-System doesn't track actual age, deeper = newer branches)
    if (branch.depth < params_.pruning_grace_period + 2) {
        return false;
    }

    // Prune if light capture is too low
    if (state.light_capture < params_.min_light_threshold) {
        return true;
    }

    // Prune if sustained resource deficit
    if (state.resource_balance < params_.min_resource_threshold &&
        state.deficit_duration >= 2) {
        return true;
    }

    return false;
}

std::vector<int> ResourceSystem::identify_pruned_branches() const {
    std::vector<int> pruned_ids;
    for (size_t i = 0; i < branch_states_.size(); ++i) {
        if (branch_states_[i].marked_for_pruning) {
            pruned_ids.push_back(static_cast<int>(i));
        }
    }
    return pruned_ids;
}

ResourceState ResourceSystem::get_state(int branch_id) const {
    if (branch_id >= 0 && branch_id < static_cast<int>(branch_states_.size())) {
        return branch_states_[branch_id];
    }
    return ResourceState{};
}

} // namespace plantgrow
