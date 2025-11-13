#pragma once

#include "camera.h"
#include "renderer.h"
#include "../core/tree.h"
#include "../core/config.h"
#include "../core/lsystem.h"
#include "../core/tropism.h"
#include <memory>
#include <string>

struct GLFWwindow;

namespace plantgrow {
namespace gui {

// Main GUI application window
class Viewer {
public:
    Viewer();
    ~Viewer();

    // Initialize viewer
    bool initialize(int width, int height, const char* title);

    // Main loop
    void run();

    // Cleanup
    void shutdown();

private:
    // GLFW window
    GLFWwindow* window_;
    int window_width_;
    int window_height_;

    // Components
    Camera camera_;
    TreeRenderer renderer_;

    // Tree data
    TreeConfig config_;
    std::unique_ptr<Tree> tree_;
    std::unique_ptr<LSystem> lsystem_;
    std::shared_ptr<TropismSystem> tropism_;  // shared_ptr to match LSystem::set_tropism()

    // UI state
    bool show_ui_;
    bool needs_regeneration_;
    std::string status_message_;
    std::string current_config_path_;

    // Mouse state
    double last_mouse_x_;
    double last_mouse_y_;
    bool mouse_dragging_;
    int mouse_button_;

    // Initialization
    bool init_glfw();
    bool init_imgui();

    // Main loop functions
    void handle_input();
    void update();
    void render();
    void render_ui();

    // UI panels
    void ui_menu_bar();
    void ui_generation_panel();
    void ui_tropism_panel();
    void ui_environment_panel();
    void ui_visualization_panel();
    void ui_export_panel();
    void ui_status_bar();

    // Tree operations
    void regenerate_tree();
    void load_config(const std::string& path);
    void save_config(const std::string& path);
    void export_usd(const std::string& path);

    // GLFW callbacks
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
};

}} // namespace plantgrow::gui
