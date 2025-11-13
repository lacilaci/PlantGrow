#pragma once

#include "../core/tree.h"
#include "../core/branch.h"
#include <vector>

namespace plantgrow {
namespace gui {

// OpenGL tree renderer
class TreeRenderer {
public:
    TreeRenderer();
    ~TreeRenderer();

    // Initialize OpenGL resources
    bool initialize();

    // Update tree data (call when tree changes)
    void update_tree(const Tree& tree);

    // Render the tree
    void render(const float* view_matrix, const float* proj_matrix);

    // Configuration
    void set_line_width(float width) { line_width_ = width; }
    void set_show_light_colors(bool show) { show_light_colors_ = show; }

private:
    // OpenGL handles
    unsigned int shader_program_;
    unsigned int vao_;  // Vertex Array Object
    unsigned int vbo_;  // Vertex Buffer Object
    unsigned int color_vbo_;  // Color buffer

    // Vertex data
    std::vector<float> vertices_;  // xyz positions
    std::vector<float> colors_;    // rgb colors
    int vertex_count_;

    // Rendering settings
    float line_width_;
    bool show_light_colors_;

    // Shader compilation
    bool compile_shaders();
    unsigned int compile_shader(const char* source, unsigned int type);

    // Helper: Extract vertices from tree
    void extract_vertices_from_tree(const Tree& tree);
};

}} // namespace plantgrow::gui
