#pragma once

#include <vector>
#include <cmath>

namespace plantgrow {

// Simple 3D vector implementation
struct Vec3 {
    float x, y, z;

    Vec3() : x(0), y(0), z(0) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    Vec3 operator+(const Vec3& other) const {
        return Vec3(x + other.x, y + other.y, z + other.z);
    }

    Vec3 operator-(const Vec3& other) const {
        return Vec3(x - other.x, y - other.y, z - other.z);
    }

    Vec3 operator*(float scalar) const {
        return Vec3(x * scalar, y * scalar, z * scalar);
    }

    Vec3 operator/(float scalar) const {
        return Vec3(x / scalar, y / scalar, z / scalar);
    }

    float length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    Vec3 normalized() const {
        float len = length();
        if (len < 1e-6f) return Vec3(0, 1, 0);
        return *this / len;
    }

    float dot(const Vec3& other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    Vec3 cross(const Vec3& other) const {
        return Vec3(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        );
    }
};

// Quaternion for rotations (will be useful for tropism)
struct Quat {
    float w, x, y, z;

    Quat() : w(1), x(0), y(0), z(0) {}
    Quat(float w, float x, float y, float z) : w(w), x(x), y(y), z(z) {}

    // Create rotation from axis and angle (radians)
    static Quat from_axis_angle(const Vec3& axis, float angle) {
        float half_angle = angle * 0.5f;
        float s = std::sin(half_angle);
        Vec3 norm_axis = axis.normalized();
        return Quat(
            std::cos(half_angle),
            norm_axis.x * s,
            norm_axis.y * s,
            norm_axis.z * s
        );
    }

    // Rotate a vector
    Vec3 rotate(const Vec3& v) const {
        Vec3 qv(x, y, z);
        Vec3 t = qv.cross(v) * 2.0f;
        return v + (t * w) + qv.cross(t);
    }
};

} // namespace plantgrow
