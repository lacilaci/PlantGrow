#include "usd_exporter.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <filesystem>
#include <system_error>

namespace plantgrow {

// Helper function to ensure parent directories exist
static bool ensure_directory_exists(const std::string& filepath) {
    std::filesystem::path path(filepath);
    if (path.has_parent_path()) {
        std::error_code ec;
        std::filesystem::create_directories(path.parent_path(), ec);
        if (ec) {
            std::cerr << "Failed to prepare export directory ("
                      << path.parent_path() << "): " << ec.message() << std::endl;
            return false;
        }
    }
    return true;
}

bool USDExporter::export_tree(const Tree& tree, const std::string& filepath) {
    // For Phase 1, we'll export as USDA (ASCII USD)
    // This is readable and doesn't require USD SDK
    return export_usda_lines(tree, filepath);
}

bool USDExporter::export_usda_lines(const Tree& tree, const std::string& filepath) {
    // Ensure output directory exists
    if (!ensure_directory_exists(filepath)) {
        return false;
    }

    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filepath << std::endl;
        return false;
    }

    // Write USD header
    file << "#usda 1.0\n";
    file << "(\n";
    file << "    defaultPrim = \"Tree\"\n";
    file << "    metersPerUnit = 1\n";
    file << "    upAxis = \"Y\"\n";
    file << ")\n\n";

    // Create main tree prim
    file << "def Xform \"Tree\" (\n";
    file << "    kind = \"component\"\n";
    file << ")\n";
    file << "{\n";

    // Collect all branches
    std::vector<Branch*> branches = const_cast<Tree&>(tree).get_all_branches();

    // Write each branch as a BasisCurves (line or curve)
    for (size_t i = 0; i < branches.size(); ++i) {
        Branch* branch = branches[i];

        // Get path points (either straight or curved)
        std::vector<Vec3> path_points = branch->get_path_points();

        file << "    def BasisCurves \"Branch_" << i << "\"\n";
        file << "    {\n";
        file << "        uniform token type = \"linear\"\n";
        file << "        uniform token basis = \"bezier\"\n";
        file << "        int[] curveVertexCounts = [" << path_points.size() << "]\n";

        // Write all points along the path
        file << "        point3f[] points = [";
        for (size_t j = 0; j < path_points.size(); ++j) {
            const Vec3& pt = path_points[j];
            file << "(" << pt.x << ", " << pt.y << ", " << pt.z << ")";
            if (j < path_points.size() - 1) {
                file << ", ";
            }
        }
        file << "]\n";

        // Write widths (taper from base to tip)
        file << "        float[] widths = [";
        for (size_t j = 0; j < path_points.size(); ++j) {
            float t = static_cast<float>(j) / (path_points.size() - 1);
            float width = branch->radius * (1.0f - t * 0.2f);  // Slight taper
            file << width;
            if (j < path_points.size() - 1) {
                file << ", ";
            }
        }
        file << "]\n";

        // Phase 2: Color based on light exposure (red=high, blue=low)
        // If light_exposure is available, use it; otherwise use depth
        float r, g, b;
        if (branch->light_exposure > 0) {
            // Light exposure coloring: red = high light, blue = low light
            float exposure = branch->light_exposure;
            r = exposure;              // High exposure = more red
            g = exposure * 0.8f;       // Medium green
            b = 1.0f - exposure;       // Low exposure = more blue
        } else {
            // Fallback: depth-based coloring (Phase 1 behavior)
            float depth_color = 1.0f - (branch->depth * 0.1f);
            if (depth_color < 0.2f) depth_color = 0.2f;
            r = depth_color;
            g = depth_color * 0.8f;
            b = depth_color * 0.6f;
        }

        file << "        color3f[] primvars:displayColor = [("
             << r << ", " << g << ", " << b << ")]\n";

        file << "    }\n\n";
    }

    file << "}\n";

    file.close();

    std::cout << "Exported tree with " << branches.size() << " branches to: " << filepath << std::endl;
    return true;
}

bool USDExporter::export_simple_format(const Tree& tree, const std::string& filepath) {
    // Ensure output directory exists
    if (!ensure_directory_exists(filepath)) {
        return false;
    }

    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filepath << std::endl;
        return false;
    }

    // Simple text format: each line is a branch
    file << "# PlantGrow Tree Export (Simple Format)\n";
    file << "# Format: branch_id parent_id start_x start_y start_z end_x end_y end_z radius depth\n\n";

    std::vector<Branch*> branches = const_cast<Tree&>(tree).get_all_branches();

    for (size_t i = 0; i < branches.size(); ++i) {
        Branch* branch = branches[i];
        Vec3 start = branch->start_pos;
        Vec3 end = branch->end_pos();

        int parent_id = -1;
        if (branch->parent) {
            // Find parent index
            for (size_t j = 0; j < branches.size(); ++j) {
                if (branches[j] == branch->parent) {
                    parent_id = static_cast<int>(j);
                    break;
                }
            }
        }

        file << i << " " << parent_id << " "
             << start.x << " " << start.y << " " << start.z << " "
             << end.x << " " << end.y << " " << end.z << " "
             << branch->radius << " " << branch->depth << "\n";
    }

    file.close();
    return true;
}

} // namespace plantgrow
