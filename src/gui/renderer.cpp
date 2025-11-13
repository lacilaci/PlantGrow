#include "renderer.h"
#include <iostream>

// OpenGL headers for macOS
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#else
#include <GL/glew.h>
#endif

namespace plantgrow {
namespace gui {

// Simple vertex shader
static const char* vertex_shader_source = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

uniform mat4 view;
uniform mat4 projection;

out vec3 vertexColor;

void main() {
    gl_Position = projection * view * vec4(aPos, 1.0);
    vertexColor = aColor;
}
)";

// Simple fragment shader
static const char* fragment_shader_source = R"(
#version 330 core
in vec3 vertexColor;
out vec4 FragColor;

void main() {
    FragColor = vec4(vertexColor, 1.0);
}
)";

TreeRenderer::TreeRenderer()
    : shader_program_(0)
    , vao_(0)
    , vbo_(0)
    , color_vbo_(0)
    , vertex_count_(0)
    , line_width_(2.0f)
    , show_light_colors_(true)
{}

TreeRenderer::~TreeRenderer() {
    if (vbo_) glDeleteBuffers(1, &vbo_);
    if (color_vbo_) glDeleteBuffers(1, &color_vbo_);
    if (vao_) glDeleteVertexArrays(1, &vao_);
    if (shader_program_) glDeleteProgram(shader_program_);
}

bool TreeRenderer::initialize() {
    // Compile shaders
    if (!compile_shaders()) {
        return false;
    }

    // Create VAO and VBOs
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glGenBuffers(1, &color_vbo_);

    glBindVertexArray(vao_);

    // Position buffer
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Color buffer
    glBindBuffer(GL_ARRAY_BUFFER, color_vbo_);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    return true;
}

bool TreeRenderer::compile_shaders() {
    // Compile vertex shader
    unsigned int vertex_shader = compile_shader(vertex_shader_source, GL_VERTEX_SHADER);
    if (!vertex_shader) return false;

    // Compile fragment shader
    unsigned int fragment_shader = compile_shader(fragment_shader_source, GL_FRAGMENT_SHADER);
    if (!fragment_shader) {
        glDeleteShader(vertex_shader);
        return false;
    }

    // Link shader program
    shader_program_ = glCreateProgram();
    glAttachShader(shader_program_, vertex_shader);
    glAttachShader(shader_program_, fragment_shader);
    glLinkProgram(shader_program_);

    // Check for linking errors
    int success;
    glGetProgramiv(shader_program_, GL_LINK_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetProgramInfoLog(shader_program_, 512, NULL, info_log);
        std::cerr << "Shader linking failed: " << info_log << std::endl;
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        return false;
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return true;
}

unsigned int TreeRenderer::compile_shader(const char* source, unsigned int type) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    // Check for compilation errors
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetShaderInfoLog(shader, 512, NULL, info_log);
        std::cerr << "Shader compilation failed: " << info_log << std::endl;
        return 0;
    }

    return shader;
}

void TreeRenderer::update_tree(const Tree& tree) {
    extract_vertices_from_tree(tree);

    // Upload to GPU
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(float), vertices_.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, color_vbo_);
    glBufferData(GL_ARRAY_BUFFER, colors_.size() * sizeof(float), colors_.data(), GL_DYNAMIC_DRAW);

    vertex_count_ = vertices_.size() / 3;
}

void TreeRenderer::extract_vertices_from_tree(const Tree& tree) {
    vertices_.clear();
    colors_.clear();

    // Get all branches
    std::vector<Branch*> branches = const_cast<Tree&>(tree).get_all_branches();

    for (Branch* branch : branches) {
        // Get path points (either straight or curved)
        std::vector<Vec3> path_points = branch->get_path_points();

        // Color based on light exposure if available
        float r, g, b;
        if (show_light_colors_ && branch->light_exposure > 0) {
            // Light exposure coloring
            float exposure = branch->light_exposure;
            r = exposure;
            g = exposure * 0.8f;
            b = 1.0f - exposure;
        } else {
            // Depth-based coloring (fallback)
            float depth_color = 1.0f - (branch->depth * 0.1f);
            if (depth_color < 0.3f) depth_color = 0.3f;
            r = depth_color * 0.6f;
            g = depth_color * 0.4f;
            b = depth_color * 0.2f;
        }

        // Create line segments between consecutive points
        for (size_t i = 0; i < path_points.size() - 1; ++i) {
            const Vec3& p1 = path_points[i];
            const Vec3& p2 = path_points[i + 1];

            // Add line segment (two vertices)
            vertices_.push_back(p1.x);
            vertices_.push_back(p1.y);
            vertices_.push_back(p1.z);

            vertices_.push_back(p2.x);
            vertices_.push_back(p2.y);
            vertices_.push_back(p2.z);

            // Add colors for both vertices
            colors_.push_back(r);
            colors_.push_back(g);
            colors_.push_back(b);

            colors_.push_back(r);
            colors_.push_back(g);
            colors_.push_back(b);
        }
    }
}

void TreeRenderer::render(const float* view_matrix, const float* proj_matrix) {
    if (vertex_count_ == 0) return;

    // Use shader program
    glUseProgram(shader_program_);

    // Set matrices
    int view_loc = glGetUniformLocation(shader_program_, "view");
    int proj_loc = glGetUniformLocation(shader_program_, "projection");
    glUniformMatrix4fv(view_loc, 1, GL_FALSE, view_matrix);
    glUniformMatrix4fv(proj_loc, 1, GL_FALSE, proj_matrix);

    // Set line width
    glLineWidth(line_width_);

    // Draw
    glBindVertexArray(vao_);
    glDrawArrays(GL_LINES, 0, vertex_count_);
    glBindVertexArray(0);
}

}} // namespace plantgrow::gui
