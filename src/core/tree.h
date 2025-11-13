#pragma once

#include "branch.h"
#include <vector>
#include <memory>

namespace plantgrow {

// Forward declaration
class ResourceSystem;

// Represents the entire tree structure
class Tree {
public:
    Tree() : age(0) {}

    // Root branch (trunk base)
    std::shared_ptr<Branch> root;

    // All branches in the tree (for easy iteration)
    std::vector<std::shared_ptr<Branch>> all_branches;

    // Tree-level properties
    int age;  // Total age in years/iterations

    // Helper methods
    void add_branch(std::shared_ptr<Branch> branch) {
        all_branches.push_back(branch);
    }

    void set_root(std::shared_ptr<Branch> branch) {
        root = branch;
        add_branch(branch);
    }

    // Collect all branches recursively (useful for traversal)
    void collect_branches_recursive(std::shared_ptr<Branch> branch, std::vector<Branch*>& out) {
        out.push_back(branch.get());
        for (auto& child : branch->children) {
            collect_branches_recursive(child, out);
        }
    }

    std::vector<Branch*> get_all_branches() {
        std::vector<Branch*> branches;
        if (root) {
            collect_branches_recursive(root, branches);
        }
        return branches;
    }

    // Phase 3: Apply resource simulation and pruning
    void apply_resource_simulation(ResourceSystem& resource_system);
};

} // namespace plantgrow
