#pragma once

#include "tree.h"
#include "tropism.h"
#include <string>
#include <map>
#include <random>
#include <stack>
#include <memory>

namespace plantgrow {

// L-System parameters
struct LSystemParams {
    std::string axiom;
    std::map<char, std::string> rules;
    int iterations;

    // Geometric interpretation
    float segment_length;
    float segment_radius;
    float branch_angle;  // In degrees
    float angle_variation;  // Random variation

    // Randomization
    unsigned int random_seed;
    float stochastic_variation;  // 0-1, probability of applying variation

    // Phase 2: Tropism support
    int curve_segments;  // Number of segments per branch for curvature (0 = straight)
};

// L-System state during interpretation
struct TurtleState {
    Vec3 position;
    Vec3 direction;
    Vec3 up;  // Up vector for local coordinate frame
    float radius;
    int depth;

    TurtleState()
        : position(0, 0, 0)
        , direction(0, 1, 0)  // Start pointing up
        , up(0, 0, 1)         // Up initially points in Z
        , radius(1.0f)
        , depth(0)
    {}
};

// L-System generator
class LSystem {
public:
    LSystem(const LSystemParams& params);

    // Set tropism system (optional, for Phase 2+)
    void set_tropism(std::shared_ptr<TropismSystem> tropism) {
        tropism_ = tropism;
    }

    // Generate L-System string
    std::string generate();

    // Interpret L-System string and create tree geometry
    Tree interpret(const std::string& lstring);

private:
    LSystemParams params_;
    std::mt19937 rng_;
    std::shared_ptr<TropismSystem> tropism_;  // Optional tropism system

    // Apply production rules for one iteration
    std::string apply_rules(const std::string& input);

    // Turtle graphics interpretation
    void process_symbol(char symbol, TurtleState& state,
                       std::stack<TurtleState>& state_stack,
                       Tree& tree,
                       std::shared_ptr<Branch>& current_branch);

    // Rotate turtle direction
    void rotate_turtle(TurtleState& state, float angle_degrees, const Vec3& axis);

    // Phase 2: Apply tropism to a branch and generate curved path
    void apply_tropism_to_branch(std::shared_ptr<Branch> branch);

    // Random number helpers
    float random_float(float min, float max);
    bool random_chance(float probability);
};

} // namespace plantgrow
