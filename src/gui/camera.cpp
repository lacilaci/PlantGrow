#include "camera.h"
#include <cmath>
#include <algorithm>

namespace plantgrow {
namespace gui {

Camera::Camera()
    : distance_(30.0f)
    , azimuth_(0.785f)  // 45 degrees
    , elevation_(0.524f)  // 30 degrees
    , target_(0.0f, 10.0f, 0.0f)
    , fov_(60.0f)
    , near_plane_(0.1f)
    , far_plane_(1000.0f)
    , orbit_sensitivity_(0.005f)
    , pan_sensitivity_(0.01f)
    , zoom_sensitivity_(1.0f)
{}

void Camera::update_orbit(float delta_x, float delta_y) {
    azimuth_ += delta_x * orbit_sensitivity_;
    elevation_ += delta_y * orbit_sensitivity_;

    // Clamp elevation to avoid gimbal lock
    const float PI = 3.14159265359f;
    elevation_ = std::clamp(elevation_, -PI/2.0f + 0.1f, PI/2.0f - 0.1f);
}

void Camera::update_pan(float delta_x, float delta_y) {
    // Calculate camera right and up vectors
    Vec3 pos = calculate_position();
    Vec3 forward = (target_ - pos).normalized();
    Vec3 right = Vec3(0, 1, 0).cross(forward).normalized();
    Vec3 up = forward.cross(right);

    // Pan target
    float pan_speed = pan_sensitivity_ * distance_;
    target_ = target_ + (right * (delta_x * pan_speed));
    target_ = target_ + (up * (-delta_y * pan_speed));
}

void Camera::update_zoom(float delta) {
    distance_ -= delta * zoom_sensitivity_;
    distance_ = std::clamp(distance_, 1.0f, 200.0f);
}

void Camera::reset() {
    distance_ = 30.0f;
    azimuth_ = 0.785f;
    elevation_ = 0.524f;
    target_ = Vec3(0.0f, 10.0f, 0.0f);
}

Vec3 Camera::calculate_position() const {
    float x = distance_ * std::cos(elevation_) * std::cos(azimuth_);
    float y = distance_ * std::sin(elevation_);
    float z = distance_ * std::cos(elevation_) * std::sin(azimuth_);

    return target_ + Vec3(x, y, z);
}

Vec3 Camera::get_position() const {
    return calculate_position();
}

void Camera::get_view_matrix(float* mat) const {
    // Simple look-at matrix construction
    Vec3 eye = calculate_position();
    Vec3 forward = (target_ - eye).normalized();
    Vec3 right = Vec3(0, 1, 0).cross(forward).normalized();
    Vec3 up = forward.cross(right);

    // Column-major matrix for OpenGL
    mat[0] = right.x;   mat[4] = right.y;   mat[8] = right.z;    mat[12] = -right.dot(eye);
    mat[1] = up.x;      mat[5] = up.y;      mat[9] = up.z;       mat[13] = -up.dot(eye);
    mat[2] = -forward.x; mat[6] = -forward.y; mat[10] = -forward.z; mat[14] = forward.dot(eye);
    mat[3] = 0;         mat[7] = 0;         mat[11] = 0;         mat[15] = 1;
}

void Camera::get_projection_matrix(float aspect, float* mat) const {
    // Perspective projection matrix
    const float PI = 3.14159265359f;
    float fov_rad = fov_ * PI / 180.0f;
    float f = 1.0f / std::tan(fov_rad / 2.0f);

    // Column-major for OpenGL
    mat[0] = f / aspect; mat[4] = 0; mat[8] = 0;  mat[12] = 0;
    mat[1] = 0; mat[5] = f; mat[9] = 0;  mat[13] = 0;
    mat[2] = 0; mat[6] = 0; mat[10] = -(far_plane_ + near_plane_) / (far_plane_ - near_plane_);
    mat[11] = -1;
    mat[3] = 0; mat[7] = 0; mat[14] = -(2.0f * far_plane_ * near_plane_) / (far_plane_ - near_plane_);
    mat[15] = 0;
}

}} // namespace plantgrow::gui
