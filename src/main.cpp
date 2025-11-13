#include "core/config.h"
#include "core/lsystem.h"
#include "core/tree.h"
#include "export/usd_exporter.h"
#include <iostream>
#include <chrono>

using namespace plantgrow;

void print_usage(const char* program_name) {
    std::cout << "PlantGrow - Procedural Tree Generation Tool\n";
    std::cout << "Usage: " << program_name << " <config.json>\n";
    std::cout << "\nExample:\n";
    std::cout << "  " << program_name << " configs/oak.json\n";
}

int main(int argc, char** argv) {
    std::cout << "=== PlantGrow - Procedural Tree Generator ===\n";
    std::cout << "Phase 1: Foundation & Basic Growth\n\n";

    // Check arguments
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    std::string config_path = argv[1];

    // Load configuration
    std::cout << "Loading configuration from: " << config_path << "\n";
    ConfigParser parser;
    TreeConfig config;

    if (!parser.parse_file(config_path, config)) {
        std::cerr << "Failed to parse configuration file\n";
        return 1;
    }

    std::cout << "\n=== Tree Generation Parameters ===\n";
    std::cout << "Species: " << config.species << "\n";
    std::cout << "Simulation years: " << config.simulation_years << "\n";
    std::cout << "Random seed: " << config.random_seed << "\n";
    std::cout << "L-System iterations: " << config.lsystem_params.iterations << "\n";
    std::cout << "Branch angle: " << config.lsystem_params.branch_angle << " degrees\n";
    std::cout << "\n";

    // Generate L-System string
    auto start_time = std::chrono::high_resolution_clock::now();

    std::cout << "Generating L-System string...\n";
    LSystem lsystem(config.lsystem_params);
    std::string lstring = lsystem.generate();

    std::cout << "L-System string generated (" << lstring.length() << " symbols)\n";
    if (lstring.length() < 200) {
        std::cout << "L-String: " << lstring << "\n";
    } else {
        std::cout << "L-String (first 200 chars): " << lstring.substr(0, 200) << "...\n";
    }
    std::cout << "\n";

    // Interpret L-System and create tree geometry
    std::cout << "Interpreting L-System and generating tree geometry...\n";
    Tree tree = lsystem.interpret(lstring);

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    std::cout << "Tree generation complete!\n";
    std::cout << "  Total branches: " << tree.all_branches.size() << "\n";
    std::cout << "  Generation time: " << duration.count() << " ms\n";
    std::cout << "\n";

    // Export to USD
    std::cout << "Exporting tree to: " << config.output_path << "\n";
    USDExporter exporter;

    if (exporter.export_tree(tree, config.output_path)) {
        std::cout << "Export successful!\n";
    } else {
        std::cerr << "Export failed!\n";
        return 1;
    }

    std::cout << "\n=== Generation Complete ===\n";
    std::cout << "Open the USD file in a compatible viewer (Houdini, Blender with USD, usdview)\n";

    return 0;
}
