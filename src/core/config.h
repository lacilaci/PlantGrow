#pragma once

#include "lsystem.h"
#include "tropism.h"
#include <string>
#include <memory>

namespace plantgrow {

// Configuration loaded from JSON file
struct TreeConfig {
    std::string species;
    int simulation_years;
    int random_seed;

    // L-System configuration
    LSystemParams lsystem_params;

    // Phase 2: Tropism configuration
    TropismParams tropism_params;
    Environment environment;
    bool tropism_enabled;

    // Output configuration
    std::string output_path;
    bool include_branches;
    bool include_foliage;

    TreeConfig() : tropism_enabled(false) {}
};

// Configuration parser
class ConfigParser {
public:
    ConfigParser() = default;

    // Parse JSON config file
    bool parse_file(const std::string& filepath, TreeConfig& config);

    // Parse JSON string
    bool parse_string(const std::string& json_str, TreeConfig& config);

private:
    // Helper to extract L-System rules
    bool parse_lsystem_rules(const std::string& json_str, TreeConfig& config);
};

} // namespace plantgrow
