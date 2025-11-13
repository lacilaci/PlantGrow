#include "usd_exporter.h"
#include <fstream>
#include <iostream>
#include <iomanip>

namespace plantgrow {

bool USDExporter::export_tree(const Tree& tree, const std::string& filepath) {
    // For Phase 1, we'll export as USDA (ASCII USD)
    // This is readable and doesn't require USD SDK
    return export_usda_lines(tree, filepath);
}

bool USDExporter::export_usda_lines(const Tree& tree, const std::string& filepath) {
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

    // Write each branch as a BasisCurves (line)
    for (size_t i = 0; i < branches.size(); ++i) {
        Branch* branch = branches[i];

        file << "    def BasisCurves \"Branch_" << i << "\"\n";
        file << "    {\n";
        file << "        uniform token type = \"linear\"\n";
        file << "        uniform token basis = \"bezier\"\n";
        file << "        int[] curveVertexCounts = [2]\n";

        // Write points (start and end of branch)
        Vec3 start = branch->start_pos;
        Vec3 end = branch->end_pos();

        file << "        point3f[] points = [";
        file << "(" << start.x << ", " << start.y << ", " << start.z << "), ";
        file << "(" << end.x << ", " << end.y << ", " << end.z << ")";
        file << "]\n";

        // Write widths (for visualization)
        file << "        float[] widths = [" << branch->radius << ", " << branch->radius * 0.8f << "]\n";

        // Write color based on depth (for debugging)
        float depth_color = 1.0f - (branch->depth * 0.1f);
        if (depth_color < 0.2f) depth_color = 0.2f;
        file << "        color3f[] primvars:displayColor = [("
             << depth_color << ", " << depth_color * 0.8f << ", " << depth_color * 0.6f << ")]\n";

        file << "    }\n\n";
    }

    file << "}\n";

    file.close();

    std::cout << "Exported tree with " << branches.size() << " branches to: " << filepath << std::endl;
    return true;
}

bool USDExporter::export_simple_format(const Tree& tree, const std::string& filepath) {
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
