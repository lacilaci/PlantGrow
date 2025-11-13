#include "viewer.h"
#include "../export/usd_exporter.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <iostream>
#include <fstream>

namespace plantgrow {
namespace gui {

Viewer::Viewer()
    : window_(nullptr)
    , window_width_(1600)
    , window_height_(1000)
    , show_ui_(true)
    , needs_regeneration_(true)
    , last_mouse_x_(0)
    , last_mouse_y_(0)
    , mouse_dragging_(false)
    , mouse_button_(0)
    , current_config_path_("configs/oak_phase2.json")
{
    // Initialize default config
    config_.species = "default";
    config_.random_seed = 12345;
    config_.simulation_years = 30;

    // L-System defaults
    config_.lsystem_params.axiom = "F";
    config_.lsystem_params.rules['F'] = "F[+F][-F]F";
    config_.lsystem_params.iterations = 5;
    config_.lsystem_params.segment_length = 1.0f;
    config_.lsystem_params.segment_radius = 0.1f;
    config_.lsystem_params.branch_angle = 30.0f;
    config_.lsystem_params.angle_variation = 10.0f;
    config_.lsystem_params.random_seed = 12345;
    config_.lsystem_params.stochastic_variation = 0.0f;
    config_.lsystem_params.curve_segments = 10;

    // Tropism defaults
    config_.tropism_enabled = true;
    config_.tropism_params.phototropism_enabled = true;
    config_.tropism_params.gravitropism_enabled = true;
    config_.tropism_params.phototropism_strength = 0.8f;
    config_.tropism_params.gravitropism_strength = 0.6f;
    config_.tropism_params.response_distance = 5.0f;
    config_.tropism_params.apical_dominance = 0.65f;

    // Environment defaults
    config_.environment.light_position = Vec3(20.0f, 100.0f, 10.0f);
    config_.environment.ambient_light = 0.2f;
    config_.environment.light_direction = config_.environment.light_position.normalized();

    config_.output_path = "output/gui_export.usda";
}

Viewer::~Viewer() {
    shutdown();
}

bool Viewer::initialize(int width, int height, const char* title) {
    window_width_ = width;
    window_height_ = height;

    if (!init_glfw()) {
        return false;
    }

    if (!init_imgui()) {
        return false;
    }

    if (!renderer_.initialize()) {
        std::cerr << "Failed to initialize renderer" << std::endl;
        return false;
    }

    // Try to load default config
    load_config(current_config_path_);

    status_message_ = "Ready";
    return true;
}

bool Viewer::init_glfw() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    // OpenGL 3.3 Core Profile for macOS
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // macOS requirement

    window_ = glfwCreateWindow(window_width_, window_height_, "PlantGrow - Tree Generator", NULL, NULL);
    if (!window_) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1); // V-Sync

    // Set callbacks
    glfwSetWindowUserPointer(window_, this);
    glfwSetFramebufferSizeCallback(window_, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window_, mouse_button_callback);
    glfwSetCursorPosCallback(window_, cursor_pos_callback);
    glfwSetScrollCallback(window_, scroll_callback);
    glfwSetKeyCallback(window_, key_callback);

    return true;
}

bool Viewer::init_imgui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window_, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    return true;
}

void Viewer::run() {
    while (!glfwWindowShouldClose(window_)) {
        handle_input();
        update();
        render();
    }
}

void Viewer::shutdown() {
    if (window_) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwDestroyWindow(window_);
        glfwTerminate();
        window_ = nullptr;
    }
}

void Viewer::handle_input() {
    glfwPollEvents();
}

void Viewer::update() {
    if (needs_regeneration_) {
        regenerate_tree();
        needs_regeneration_ = false;
    }
}

void Viewer::render() {
    // Clear
    glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    // Get viewport size
    int display_w, display_h;
    glfwGetFramebufferSize(window_, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);

    // Render tree
    if (tree_) {
        float view[16], proj[16];
        camera_.get_view_matrix(view);
        camera_.get_projection_matrix((float)display_w / display_h, proj);
        renderer_.render(view, proj);
    }

    // Render UI
    render_ui();

    glfwSwapBuffers(window_);
}

void Viewer::render_ui() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (show_ui_) {
        ui_menu_bar();
        ui_generation_panel();
        ui_tropism_panel();
        ui_environment_panel();
        ui_visualization_panel();
        ui_export_panel();
        ui_status_bar();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

// Continued in next message due to length...

}} // namespace plantgrow::gui
// UI PANEL IMPLEMENTATIONS FOR viewer.cpp
// Add these methods to the Viewer class

void Viewer::ui_menu_bar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Load Config...", "Ctrl+O")) {
                // For now, just use current path
                load_config(current_config_path_);
            }
            if (ImGui::MenuItem("Save Config...", "Ctrl+S")) {
                save_config(current_config_path_);
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Export USD...", "Ctrl+E")) {
                export_usd(config_.output_path);
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Quit", "Ctrl+Q")) {
                glfwSetWindowShouldClose(window_, true);
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Show UI", "U", &show_ui_);
            if (ImGui::MenuItem("Reset Camera", "R")) {
                camera_.reset();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void Viewer::ui_generation_panel() {
    ImGui::Begin("Generation");
    
    ImGui::Text("L-System");
    ImGui::Separator();
    
    if (ImGui::SliderInt("Iterations", &config_.lsystem_params.iterations, 1, 8)) {
        needs_regeneration_ = true;
    }
    
    if (ImGui::SliderFloat("Branch Angle", &config_.lsystem_params.branch_angle, 10.0f, 90.0f, "%.1f deg")) {
        needs_regeneration_ = true;
    }
    
    if (ImGui::SliderFloat("Angle Variation", &config_.lsystem_params.angle_variation, 0.0f, 30.0f, "%.1f deg")) {
        needs_regeneration_ = true;
    }
    
    ImGui::Spacing();
    if (ImGui::InputInt("Random Seed", &config_.random_seed)) {
        config_.lsystem_params.random_seed = config_.random_seed;
        needs_regeneration_ = true;
    }
    
    ImGui::Spacing();
    if (ImGui::Button("Regenerate Tree")) {
        needs_regeneration_ = true;
    }
    
    ImGui::End();
}

void Viewer::ui_tropism_panel() {
    ImGui::Begin("Tropism");
    
    if (ImGui::Checkbox("Enable Tropism", &config_.tropism_enabled)) {
        needs_regeneration_ = true;
    }
    
    if (config_.tropism_enabled) {
        ImGui::Separator();
        
        if (ImGui::Checkbox("Phototropism", &config_.tropism_params.phototropism_enabled)) {
            needs_regeneration_ = true;
        }
        if (config_.tropism_params.phototropism_enabled) {
            if (ImGui::SliderFloat("Photo Strength", &config_.tropism_params.phototropism_strength, 0.0f, 1.0f)) {
                needs_regeneration_ = true;
            }
        }
        
        ImGui::Spacing();
        if (ImGui::Checkbox("Gravitropism", &config_.tropism_params.gravitropism_enabled)) {
            needs_regeneration_ = true;
        }
        if (config_.tropism_params.gravitropism_enabled) {
            if (ImGui::SliderFloat("Gravi Strength", &config_.tropism_params.gravitropism_strength, 0.0f, 1.0f)) {
                needs_regeneration_ = true;
            }
        }
        
        ImGui::Spacing();
        if (ImGui::SliderFloat("Apical Dominance", &config_.tropism_params.apical_dominance, 0.0f, 1.0f)) {
            needs_regeneration_ = true;
        }
        
        if (ImGui::SliderInt("Curve Segments", &config_.lsystem_params.curve_segments, 0, 20)) {
            needs_regeneration_ = true;
        }
    }
    
    ImGui::End();
}

void Viewer::ui_environment_panel() {
    ImGui::Begin("Environment");
    
    ImGui::Text("Light Source");
    ImGui::Separator();
    
    bool changed = false;
    changed |= ImGui::SliderFloat("Light X", &config_.environment.light_position.x, -100.0f, 100.0f);
    changed |= ImGui::SliderFloat("Light Y", &config_.environment.light_position.y, 0.0f, 200.0f);
    changed |= ImGui::SliderFloat("Light Z", &config_.environment.light_position.z, -100.0f, 100.0f);
    
    if (changed) {
        config_.environment.light_direction = config_.environment.light_position.normalized();
        needs_regeneration_ = true;
    }
    
    if (ImGui::SliderFloat("Ambient Light", &config_.environment.ambient_light, 0.0f, 1.0f)) {
        needs_regeneration_ = true;
    }
    
    ImGui::End();
}

void Viewer::ui_visualization_panel() {
    ImGui::Begin("Visualization");
    
    static float line_width = 2.0f;
    if (ImGui::SliderFloat("Line Width", &line_width, 0.5f, 5.0f)) {
        renderer_.set_line_width(line_width);
    }
    
    static bool show_light_colors = true;
    if (ImGui::Checkbox("Light Exposure Colors", &show_light_colors)) {
        renderer_.set_show_light_colors(show_light_colors);
        if (tree_) {
            renderer_.update_tree(*tree_);
        }
    }
    
    ImGui::Separator();
    ImGui::Text("Camera");
    ImGui::Text("Distance: %.1f", camera_.get_distance());
    ImGui::Text("Azimuth: %.2f rad", camera_.get_azimuth());
    ImGui::Text("Elevation: %.2f rad", camera_.get_elevation());
    
    if (ImGui::Button("Reset Camera")) {
        camera_.reset();
    }
    
    ImGui::End();
}

void Viewer::ui_export_panel() {
    ImGui::Begin("Export");
    
    static char path_buffer[256] = "output/gui_export.usda";
    ImGui::InputText("USD Path", path_buffer, sizeof(path_buffer));
    
    if (ImGui::Button("Export to USD")) {
        export_usd(path_buffer);
    }
    
    if (tree_) {
        ImGui::Separator();
        ImGui::Text("Tree Statistics");
        ImGui::Text("Total Branches: %zu", tree_->all_branches.size());
    }
    
    ImGui::End();
}

void Viewer::ui_status_bar() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + viewport->Size.y - 30));
    ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, 30));
    
    ImGui::Begin("StatusBar", nullptr, 
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);
    
    ImGui::Text("Status: %s", status_message_.c_str());
    ImGui::SameLine(ImGui::GetWindowWidth() - 200);
    ImGui::Text("Config: %s", current_config_path_.c_str());
    
    ImGui::End();
}

// TREE OPERATIONS

void Viewer::regenerate_tree() {
    status_message_ = "Generating tree...";
    
    // Create L-System
    lsystem_ = std::make_unique<LSystem>(config_.lsystem_params);
    
    // Set up tropism if enabled
    if (config_.tropism_enabled) {
        tropism_ = std::make_shared<TropismSystem>(config_.tropism_params, config_.environment);
        lsystem_->set_tropism(tropism_);
    } else {
        lsystem_->set_tropism(nullptr);
    }
    
    // Generate L-string
    std::string lstring = lsystem_->generate();
    
    // Interpret and create tree
    tree_ = std::make_unique<Tree>(lsystem_->interpret(lstring));
    
    // Update renderer
    renderer_.update_tree(*tree_);
    
    status_message_ = "Tree generated";
}

void Viewer::load_config(const std::string& path) {
    ConfigParser parser;
    if (parser.parse_file(path, config_)) {
        current_config_path_ = path;
        needs_regeneration_ = true;
        status_message_ = "Config loaded: " + path;
    } else {
        status_message_ = "Failed to load config: " + path;
    }
}

void Viewer::save_config(const std::string& path) {
    // TODO: Implement JSON serialization
    status_message_ = "Config save not yet implemented";
}

void Viewer::export_usd(const std::string& path) {
    if (!tree_) {
        status_message_ = "No tree to export";
        return;
    }
    
    USDExporter exporter;
    if (exporter.export_tree(*tree_, path)) {
        status_message_ = "Exported to: " + path;
    } else {
        status_message_ = "Export failed";
    }
}

// GLFW CALLBACKS

void Viewer::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    Viewer* viewer = static_cast<Viewer*>(glfwGetWindowUserPointer(window));
    viewer->window_width_ = width;
    viewer->window_height_ = height;
}

void Viewer::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    Viewer* viewer = static_cast<Viewer*>(glfwGetWindowUserPointer(window));
    
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) return;  // ImGui is using the mouse
    
    if (action == GLFW_PRESS) {
        viewer->mouse_dragging_ = true;
        viewer->mouse_button_ = button;
        glfwGetCursorPos(window, &viewer->last_mouse_x_, &viewer->last_mouse_y_);
    } else if (action == GLFW_RELEASE) {
        viewer->mouse_dragging_ = false;
    }
}

void Viewer::cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
    Viewer* viewer = static_cast<Viewer*>(glfwGetWindowUserPointer(window));
    
    if (!viewer->mouse_dragging_) return;
    
    float delta_x = static_cast<float>(xpos - viewer->last_mouse_x_);
    float delta_y = static_cast<float>(ypos - viewer->last_mouse_y_);
    
    if (viewer->mouse_button_ == GLFW_MOUSE_BUTTON_LEFT) {
        // Orbit
        viewer->camera_.update_orbit(delta_x, delta_y);
    } else if (viewer->mouse_button_ == GLFW_MOUSE_BUTTON_RIGHT) {
        // Pan
        viewer->camera_.update_pan(delta_x, delta_y);
    }
    
    viewer->last_mouse_x_ = xpos;
    viewer->last_mouse_y_ = ypos;
}

void Viewer::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    Viewer* viewer = static_cast<Viewer*>(glfwGetWindowUserPointer(window));
    
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) return;
    
    viewer->camera_.update_zoom(static_cast<float>(-yoffset));
}

void Viewer::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Viewer* viewer = static_cast<Viewer*>(glfwGetWindowUserPointer(window));
    
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_R:
                viewer->camera_.reset();
                break;
            case GLFW_KEY_U:
                viewer->show_ui_ = !viewer->show_ui_;
                break;
            case GLFW_KEY_SPACE:
                viewer->needs_regeneration_ = true;
                break;
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window, true);
                break;
        }
    }
}

