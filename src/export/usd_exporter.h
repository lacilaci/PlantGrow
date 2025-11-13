#pragma once

#include "../core/tree.h"
#include <string>

namespace plantgrow {

class USDExporter {
public:
    USDExporter() = default;

    // Export tree as USD file (or fallback format)
    bool export_tree(const Tree& tree, const std::string& filepath);

private:
    // Export as USDA (ASCII USD) for Phase 1 - simple lines
    bool export_usda_lines(const Tree& tree, const std::string& filepath);

    // Fallback: Export as simple text format if USD not available
    bool export_simple_format(const Tree& tree, const std::string& filepath);
};

} // namespace plantgrow
