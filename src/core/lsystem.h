#pragma once

#include "tree.h"
#include <string>
#include <map>
#include <random>
#include <stack>

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

    // Generate L-System string
    std::string generate();

    // Interpret L-System string and create tree geometry
    Tree interpret(const std::string& lstring);

private:
    LSystemParams params_;
    std::mt19937 rng_;

    // Apply production rules for one iteration
    std::string apply_rules(const std::string& input);

    // Turtle graphics interpretation
    void process_symbol(char symbol, TurtleState& state,
                       std::stack<TurtleState>& state_stack,
                       Tree& tree,
                       std::shared_ptr<Branch>& current_branch);

    // Rotate turtle direction
    void rotate_turtle(TurtleState& state, float angle_degrees, const Vec3& axis);

    // Random number helpers
    float random_float(float min, float max);
    bool random_chance(float probability);
};

} // namespace plantgrow
