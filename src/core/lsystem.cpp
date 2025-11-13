#include "lsystem.h"
#include <iostream>
#include <cmath>

namespace plantgrow {

LSystem::LSystem(const LSystemParams& params)
    : params_(params)
    , rng_(params.random_seed)
{}

std::string LSystem::generate() {
    std::string current = params_.axiom;

    for (int i = 0; i < params_.iterations; ++i) {
        current = apply_rules(current);
    }

    return current;
}

std::string LSystem::apply_rules(const std::string& input) {
    std::string output;

    for (char c : input) {
        // Check if there's a rule for this symbol
        auto it = params_.rules.find(c);
        if (it != params_.rules.end()) {
            // Apply stochastic variation if enabled
            if (params_.stochastic_variation > 0 && random_chance(params_.stochastic_variation)) {
                // Add slight variation to rule application
                output += it->second;
            } else {
                output += it->second;
            }
        } else {
            // No rule, keep the symbol as-is
            output += c;
        }
    }

    return output;
}

Tree LSystem::interpret(const std::string& lstring) {
    Tree tree;
    TurtleState state;
    std::stack<TurtleState> state_stack;
    std::shared_ptr<Branch> current_branch = nullptr;

    // Create root branch
    auto root = std::make_shared<Branch>(
        Vec3(0, 0, 0),
        Vec3(0, 1, 0),
        params_.segment_length,
        params_.segment_radius
    );
    tree.set_root(root);
    current_branch = root;
    state.position = root->end_pos();

    // Interpret each symbol
    for (char symbol : lstring) {
        process_symbol(symbol, state, state_stack, tree, current_branch);
    }

    return tree;
}

void LSystem::process_symbol(char symbol, TurtleState& state,
                             std::stack<TurtleState>& state_stack,
                             Tree& tree,
                             std::shared_ptr<Branch>& current_branch) {
    const float PI = 3.14159265359f;

    switch (symbol) {
        case 'F':  // Draw forward (create branch segment)
        {
            float length = params_.segment_length;
            float radius = params_.segment_radius * std::pow(0.95f, state.depth);

            auto new_branch = std::make_shared<Branch>(
                state.position,
                state.direction,
                length,
                radius
            );

            new_branch->depth = state.depth;

            if (current_branch) {
                current_branch->add_child(new_branch);
            }

            tree.add_branch(new_branch);
            current_branch = new_branch;
            state.position = new_branch->end_pos();
            break;
        }

        case 'f':  // Move forward without drawing
        {
            state.position = state.position + (state.direction * params_.segment_length);
            break;
        }

        case '+':  // Rotate right around up axis
        {
            float angle = params_.branch_angle + random_float(-params_.angle_variation, params_.angle_variation);
            rotate_turtle(state, angle, state.up);
            break;
        }

        case '-':  // Rotate left around up axis
        {
            float angle = -(params_.branch_angle + random_float(-params_.angle_variation, params_.angle_variation));
            rotate_turtle(state, angle, state.up);
            break;
        }

        case '&':  // Pitch down
        {
            Vec3 right = state.direction.cross(state.up).normalized();
            float angle = params_.branch_angle + random_float(-params_.angle_variation, params_.angle_variation);
            rotate_turtle(state, angle, right);
            break;
        }

        case '^':  // Pitch up
        {
            Vec3 right = state.direction.cross(state.up).normalized();
            float angle = -(params_.branch_angle + random_float(-params_.angle_variation, params_.angle_variation));
            rotate_turtle(state, angle, right);
            break;
        }

        case '\\':  // Roll right
        {
            float angle = params_.branch_angle;
            rotate_turtle(state, angle, state.direction);
            break;
        }

        case '/':  // Roll left
        {
            float angle = -params_.branch_angle;
            rotate_turtle(state, angle, state.direction);
            break;
        }

        case '[':  // Push state (start branch)
        {
            state_stack.push(state);
            state.depth++;
            break;
        }

        case ']':  // Pop state (end branch)
        {
            if (!state_stack.empty()) {
                state = state_stack.top();
                state_stack.pop();

                // Find the branch at this position to make it current
                // For Phase 1, we'll traverse from root
                for (auto& branch : tree.all_branches) {
                    if ((branch->end_pos() - state.position).length() < 0.001f) {
                        current_branch = branch;
                        break;
                    }
                }
            }
            break;
        }

        default:
            // Ignore unknown symbols
            break;
    }
}

void LSystem::rotate_turtle(TurtleState& state, float angle_degrees, const Vec3& axis) {
    const float PI = 3.14159265359f;
    float angle_radians = angle_degrees * PI / 180.0f;

    Quat rotation = Quat::from_axis_angle(axis, angle_radians);
    state.direction = rotation.rotate(state.direction).normalized();
    state.up = rotation.rotate(state.up).normalized();
}

float LSystem::random_float(float min, float max) {
    std::uniform_real_distribution<float> dist(min, max);
    return dist(rng_);
}

bool LSystem::random_chance(float probability) {
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    return dist(rng_) < probability;
}

} // namespace plantgrow
