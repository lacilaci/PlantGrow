#include "tree.h"
#include "resources.h"
#include <algorithm>
#include <iostream>

namespace plantgrow {

// Implementation file for Tree class
// Most functionality is inline in the header for Phase 1
// This file is here for future implementations

void Tree::apply_resource_simulation(ResourceSystem& resource_system) {
    if (all_branches.empty()) {
        return;
    }

    // Run resource simulation
    resource_system.calculate_resources(all_branches);

    // Get pruned branches
    std::vector<int> pruned_ids = resource_system.identify_pruned_branches();

    if (!pruned_ids.empty()) {
        std::cout << "Pruning " << pruned_ids.size() << " branches due to resource deficit" << std::endl;

        // Mark branches for removal
        // Note: We remove branches from parent's children list but keep them in all_branches
        // for visualization purposes (they'll just not be rendered or could be rendered differently)
        for (int id : pruned_ids) {
            if (id >= 0 && id < static_cast<int>(all_branches.size())) {
                auto& branch = all_branches[id];

                // Find and remove this branch from its parent's children
                if (branch->parent) {
                    auto& siblings = branch->parent->children;
                    siblings.erase(
                        std::remove_if(siblings.begin(), siblings.end(),
                            [&branch](const std::shared_ptr<Branch>& child) {
                                return child.get() == branch.get();
                            }),
                        siblings.end()
                    );
                }

                // Mark as pruned by setting a flag (we can add this to Branch if needed)
                // For now, we just remove it from the hierarchy
            }
        }

        // Remove pruned branches from all_branches vector
        std::vector<std::shared_ptr<Branch>> surviving_branches;
        for (size_t i = 0; i < all_branches.size(); ++i) {
            if (std::find(pruned_ids.begin(), pruned_ids.end(), static_cast<int>(i)) == pruned_ids.end()) {
                surviving_branches.push_back(all_branches[i]);
            }
        }
        all_branches = std::move(surviving_branches);

        std::cout << "  Remaining branches: " << all_branches.size() << std::endl;
    }
}

} // namespace plantgrow
