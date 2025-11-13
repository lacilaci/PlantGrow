#pragma once

#include "../core/types.h"

namespace plantgrow {
namespace gui {

// Orbit camera for tree visualization
class Camera {
public:
    Camera();

    // Update camera with mouse input
    void update_orbit(float delta_x, float delta_y);
    void update_pan(float delta_x, float delta_y);
    void update_zoom(float delta);

    // Reset camera to default position
    void reset();

    // Get view and projection matrices
    void get_view_matrix(float* mat) const;
    void get_projection_matrix(float aspect, float* mat) const;

    // Get camera position (for lighting calculations)
    Vec3 get_position() const;

    // Configuration
    void set_target(const Vec3& target) { target_ = target; }
    Vec3 get_target() const { return target_; }

    float get_distance() const { return distance_; }
    float get_azimuth() const { return azimuth_; }
    float get_elevation() const { return elevation_; }

private:
    // Orbit parameters
    float distance_;      // Distance from target
    float azimuth_;       // Horizontal angle (radians)
    float elevation_;     // Vertical angle (radians)
    Vec3 target_;         // Look-at point

    // Camera settings
    float fov_;           // Field of view (degrees)
    float near_plane_;
    float far_plane_;

    // Sensitivity
    float orbit_sensitivity_;
    float pan_sensitivity_;
    float zoom_sensitivity_;

    // Helper: Calculate camera position from orbit parameters
    Vec3 calculate_position() const;
};

}} // namespace plantgrow::gui
