#include "config.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

namespace plantgrow {

// Simple JSON value extractor (for Phase 1)
// In production, we'd use nlohmann/json or similar
namespace json_utils {

std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r\"");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r\"");
    return str.substr(first, last - first + 1);
}

std::string get_value(const std::string& json, const std::string& key) {
    std::string search = "\"" + key + "\"";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return "";

    pos = json.find(":", pos);
    if (pos == std::string::npos) return "";

    pos++;
    size_t end = json.find_first_of(",}\n", pos);
    if (end == std::string::npos) end = json.length();

    return trim(json.substr(pos, end - pos));
}

int get_int(const std::string& json, const std::string& key, int default_val = 0) {
    std::string val = get_value(json, key);
    if (val.empty()) return default_val;
    try {
        return std::stoi(val);
    } catch (...) {
        return default_val;
    }
}

float get_float(const std::string& json, const std::string& key, float default_val = 0.0f) {
    std::string val = get_value(json, key);
    if (val.empty()) return default_val;
    try {
        return std::stof(val);
    } catch (...) {
        return default_val;
    }
}

std::string get_string(const std::string& json, const std::string& key, const std::string& default_val = "") {
    std::string val = get_value(json, key);
    return val.empty() ? default_val : val;
}

std::string get_object(const std::string& json, const std::string& key) {
    std::string search = "\"" + key + "\"";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return "";

    pos = json.find("{", pos);
    if (pos == std::string::npos) return "";

    int brace_count = 1;
    size_t start = pos + 1;
    pos++;

    while (pos < json.length() && brace_count > 0) {
        if (json[pos] == '{') brace_count++;
        else if (json[pos] == '}') brace_count--;
        pos++;
    }

    return json.substr(start, pos - start - 1);
}

} // namespace json_utils

bool ConfigParser::parse_file(const std::string& filepath, TreeConfig& config) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open config file: " << filepath << std::endl;
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return parse_string(buffer.str(), config);
}

bool ConfigParser::parse_string(const std::string& json_str, TreeConfig& config) {
    using namespace json_utils;

    // Parse top-level fields
    config.species = get_string(json_str, "species", "unknown");

    // Parse growth_parameters section
    std::string growth_params = get_object(json_str, "growth_parameters");
    if (!growth_params.empty()) {
        config.simulation_years = get_int(growth_params, "simulation_years", 10);
        config.random_seed = get_int(growth_params, "random_seed", 12345);
    } else {
        config.simulation_years = 10;
        config.random_seed = 12345;
    }

    // Parse l_system section
    std::string lsystem = get_object(json_str, "l_system");
    if (!lsystem.empty()) {
        config.lsystem_params.axiom = get_string(lsystem, "axiom", "F");
        config.lsystem_params.iterations = get_int(lsystem, "iterations", 5);
        config.lsystem_params.stochastic_variation = get_float(lsystem, "stochastic_variation", 0.0f);

        // Parse rules object
        std::string rules_obj = get_object(lsystem, "rules");
        if (!rules_obj.empty()) {
            // Simple rule parsing: look for "F": "..."
            std::string f_rule = get_string(rules_obj, "F", "FF");
            config.lsystem_params.rules['F'] = f_rule;
        }
    }

    // Parse branching section
    std::string branching = get_object(json_str, "branching");
    if (!branching.empty()) {
        config.lsystem_params.branch_angle = get_float(branching, "base_angle_degrees", 25.0f);
        config.lsystem_params.angle_variation = get_float(branching, "angle_variation", 5.0f);
    } else {
        config.lsystem_params.branch_angle = 25.0f;
        config.lsystem_params.angle_variation = 5.0f;
    }

    // Set default geometric parameters
    config.lsystem_params.segment_length = 1.0f;
    config.lsystem_params.segment_radius = 0.1f;
    config.lsystem_params.random_seed = config.random_seed;
    config.lsystem_params.curve_segments = 0;  // Default: straight branches (Phase 1 behavior)

    // Phase 2: Parse tropism section
    std::string tropism = get_object(json_str, "tropism");
    if (!tropism.empty()) {
        config.tropism_enabled = true;
        config.tropism_params.phototropism_strength = get_float(tropism, "phototropism_strength", 0.8f);
        config.tropism_params.gravitropism_strength = get_float(tropism, "gravitropism_strength", 0.6f);
        config.tropism_params.response_distance = get_float(tropism, "response_distance", 5.0f);
        config.tropism_params.apical_dominance = get_float(tropism, "apical_dominance", 0.65f);

        // Check if individual tropisms are enabled
        std::string photo_enabled = get_string(tropism, "phototropism_enabled", "true");
        std::string gravi_enabled = get_string(tropism, "gravitropism_enabled", "true");
        config.tropism_params.phototropism_enabled = (photo_enabled == "true");
        config.tropism_params.gravitropism_enabled = (gravi_enabled == "true");

        // Set curve segments if tropism is enabled
        config.lsystem_params.curve_segments = get_int(tropism, "curve_segments", 10);
    } else {
        config.tropism_enabled = false;
        config.lsystem_params.curve_segments = 0;
    }

    // Parse environment section
    std::string env = get_object(json_str, "environment");
    if (!env.empty()) {
        // Light position
        config.environment.light_position.x = get_float(env, "light_x", 0.0f);
        config.environment.light_position.y = get_float(env, "light_y", 100.0f);
        config.environment.light_position.z = get_float(env, "light_z", 0.0f);

        // Ambient light
        config.environment.ambient_light = get_float(env, "ambient_light", 0.2f);

        // Update light direction based on position
        config.environment.light_direction = config.environment.light_position.normalized();
    }

    // Phase 3: Parse resources section
    std::string resources = get_object(json_str, "resources");
    if (!resources.empty()) {
        config.resource_simulation_enabled = true;

        // Light capture parameters
        std::string light_comp_str = get_string(resources, "light_competition_enabled", "true");
        config.resource_params.light_competition_enabled = (light_comp_str == "true");
        config.resource_params.base_light_level = get_float(resources, "base_light_level", 1.0f);
        config.resource_params.occlusion_radius = get_float(resources, "occlusion_radius", 2.0f);
        config.resource_params.occlusion_falloff = get_float(resources, "occlusion_falloff", 0.5f);

        // Resource allocation parameters
        config.resource_params.photosynthesis_efficiency = get_float(resources, "photosynthesis_efficiency", 1.0f);
        config.resource_params.resource_transport_rate = get_float(resources, "resource_transport_rate", 0.8f);
        config.resource_params.maintenance_cost = get_float(resources, "maintenance_cost", 0.1f);

        // Pruning parameters
        std::string pruning_str = get_string(resources, "pruning_enabled", "true");
        config.resource_params.pruning_enabled = (pruning_str == "true");
        config.resource_params.min_light_threshold = get_float(resources, "min_light_threshold", 0.15f);
        config.resource_params.min_resource_threshold = get_float(resources, "min_resource_threshold", 0.2f);
        config.resource_params.pruning_grace_period = get_int(resources, "pruning_grace_period", 2);

        // Competition parameters
        config.resource_params.competition_radius = get_float(resources, "competition_radius", 1.5f);
        config.resource_params.dominance_factor = get_float(resources, "dominance_factor", 0.7f);
    } else {
        config.resource_simulation_enabled = false;
    }

    // Parse output section
    std::string output = get_object(json_str, "output");
    if (!output.empty()) {
        config.output_path = get_string(output, "usd_path", "output/tree.usd");
        std::string inc_branches = get_string(output, "include_branches", "true");
        std::string inc_foliage = get_string(output, "include_foliage", "false");
        config.include_branches = (inc_branches == "true");
        config.include_foliage = (inc_foliage == "true");
    } else {
        config.output_path = "output/tree.usd";
        config.include_branches = true;
        config.include_foliage = false;
    }

    std::cout << "Loaded config for species: " << config.species << std::endl;
    std::cout << "  L-System axiom: " << config.lsystem_params.axiom << std::endl;
    std::cout << "  L-System iterations: " << config.lsystem_params.iterations << std::endl;
    std::cout << "  Branch angle: " << config.lsystem_params.branch_angle << " degrees" << std::endl;
    std::cout << "  Tropism enabled: " << (config.tropism_enabled ? "yes" : "no") << std::endl;
    if (config.tropism_enabled) {
        std::cout << "    Curve segments: " << config.lsystem_params.curve_segments << std::endl;
        std::cout << "    Phototropism: " << config.tropism_params.phototropism_strength << std::endl;
        std::cout << "    Gravitropism: " << config.tropism_params.gravitropism_strength << std::endl;
    }
    std::cout << "  Resource simulation enabled: " << (config.resource_simulation_enabled ? "yes" : "no") << std::endl;
    if (config.resource_simulation_enabled) {
        std::cout << "    Pruning: " << (config.resource_params.pruning_enabled ? "enabled" : "disabled") << std::endl;
        std::cout << "    Light competition: " << (config.resource_params.light_competition_enabled ? "enabled" : "disabled") << std::endl;
    }
    std::cout << "  Output path: " << config.output_path << std::endl;

    return true;
}

} // namespace plantgrow
