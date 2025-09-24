#pragma once
#include <cmath>
#include <initializer_list>

// --------------------------
// Vec2
// --------------------------
struct vec2 {
    float x, y;
    vec2(float _x = 0, float _y = 0) : x(_x), y(_y) {}
    vec2 operator+(const vec2& v) const { return { x + v.x, y + v.y }; }
    vec2 operator-(const vec2& v) const { return { x - v.x, y - v.y }; }
    vec2 operator*(float s) const { return { x * s, y * s }; }
    float dot(const vec2& v) const { return x * v.x + y * v.y; }
    float length() const { return std::sqrt(x * x + y * y); }
    vec2 normalized() const { float l = length(); return l > 0 ? *this * (1.0f / l) : *this; }
};

// --------------------------
// Vec3
// --------------------------
struct vec3 {
    float x, y, z;
    vec3(float _x = 0, float _y = 0, float _z = 0) : x(_x), y(_y), z(_z) {}
    vec3 operator+(const vec3& v) const { return { x + v.x, y + v.y, z + v.z }; }
    vec3 operator-(const vec3& v) const { return { x - v.x, y - v.y, z - v.z }; }
    vec3 operator*(float s) const { return { x * s, y * s, z * s }; }
    float dot(const vec3& v) const { return x * v.x + y * v.y + z * v.z; }
    vec3 cross(const vec3& v) const {
        return { y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x };
    }
    float length() const { return std::sqrt(x * x + y * y + z * z); }
    vec3 normalized() const { float l = length(); return l > 0 ? *this * (1.0f / l) : *this; }
};

// --------------------------
// Vec4
// --------------------------
struct vec4 {
    float x, y, z, w;
    vec4(float _x = 0, float _y = 0, float _z = 0, float _w = 0) : x(_x), y(_y), z(_z), w(_w) {}
    vec4 operator+(const vec4& v) const { return { x + v.x, y + v.y, z + v.z, w + v.w }; }
    vec4 operator-(const vec4& v) const { return { x - v.x, y - v.y, z - v.z, w - v.w }; }
    vec4 operator*(float s) const { return { x * s, y * s, z * s, w * s }; }
    float dot(const vec4& v) const { return x * v.x + y * v.y + z * v.z + w * v.w; }
};

// ===================
// Mat2 (2x2)
// ===================
struct mat2 {
    float m[4]; // column-major
    mat2(float v = 1.0f) { // Identity * v
        for (int i = 0; i < 4; ++i) m[i] = 0;
        m[0] = m[3] = v;
    }
    float& operator()(int row, int col) { return m[col * 2 + row]; }
    const float& operator()(int row, int col) const { return m[col * 2 + row]; }
    mat2 operator*(const mat2& o) const {
        mat2 r(0.0f);
        for (int c = 0; c < 2; ++c)
            for (int r_ = 0; r_ < 2; ++r_)
                for (int k = 0; k < 2; ++k)
                    r(r_, c) += (*this)(r_, k) * o(k, c);
        return r;
    }
    vec2 operator*(const vec2& v) const {
        return {
            m[0] * v.x + m[2] * v.y,
            m[1] * v.x + m[3] * v.y
        };
    }
    static mat2 scale(float sx, float sy) {
        mat2 r(1.0f);
        r(0, 0) = sx; r(1, 1) = sy;
        return r;
    }
};

// ===================
// Mat3 (3x3)
// ===================
struct mat3 {
    float m[9]; // column-major
    mat3(float v = 1.0f) { // Identity * v
        for (int i = 0; i < 9; ++i) m[i] = 0;
        m[0] = m[4] = m[8] = v;
    }
    float& operator()(int row, int col) { return m[col * 3 + row]; }
    const float& operator()(int row, int col) const { return m[col * 3 + row]; }
    mat3 operator*(const mat3& o) const {
        mat3 r(0.0f);
        for (int c = 0; c < 3; ++c)
            for (int r_ = 0; r_ < 3; ++r_)
                for (int k = 0; k < 3; ++k)
                    r(r_, c) += (*this)(r_, k) * o(k, c);
        return r;
    }
    vec3 operator*(const vec3& v) const {
        return {
            m[0] * v.x + m[3] * v.y + m[6] * v.z,
            m[1] * v.x + m[4] * v.y + m[7] * v.z,
            m[2] * v.x + m[5] * v.y + m[8] * v.z
        };
    }
    static mat3 scale(float sx, float sy, float sz) {
        mat3 r(1.0f);
        r(0, 0) = sx; r(1, 1) = sy; r(2, 2) = sz;
        return r;
    }
    static mat3 translate(float tx, float ty) {
        mat3 r(1.0f);
        r(0, 2) = tx; r(1, 2) = ty;
        return r;
    }
};

// ===================

// --------------------------
// Mat4
// --------------------------
struct mat4 {
    float m[16]; // column-major (OpenGL style)
    mat4(float v = 1.0f) { // Identity * v
        for (int i = 0; i < 16; ++i) m[i] = 0;
        m[0] = m[5] = m[10] = m[15] = v;
    }
    float& operator()(int row, int col) { return m[col * 4 + row]; }
    const float& operator()(int row, int col) const { return m[col * 4 + row]; }
    // Matrix multiply
    mat4 operator*(const mat4& o) const {
        mat4 r(0.0f);
        for (int c = 0; c < 4; ++c)
            for (int r_ = 0; r_ < 4; ++r_)
                for (int k = 0; k < 4; ++k)
                    r(r_, c) += (*this)(r_, k) * o(k, c);
        return r;
    }
    // Apply to vec4
    vec4 operator*(const vec4& v) const {
        vec4 r;
        for (int r_ = 0; r_ < 4; ++r_)
            r.x += (*this)(r_, 0) * v.x,
            r.y += (*this)(r_, 1) * v.y,
            r.z += (*this)(r_, 2) * v.z,
            r.w += (*this)(r_, 3) * v.w;
        return r;
    }
    // Translation
    static mat4 translate(float tx, float ty, float tz) {
        mat4 r(1.0f);
        r(0, 3) = tx; r(1, 3) = ty; r(2, 3) = tz;
        return r;
    }
    // Scale
    static mat4 scale(float sx, float sy, float sz) {
        mat4 r(1.0f);
        r(0, 0) = sx; r(1, 1) = sy; r(2, 2) = sz;
        return r;
    }
    // Orthographic
    static mat4 ortho(float left, float right, float bottom, float top, float near, float far) {
        mat4 r(1.0f);
        r(0, 0) = 2 / (right - left);
        r(1, 1) = 2 / (top - bottom);
        r(2, 2) = -2 / (far - near);
        r(0, 3) = -(right + left) / (right - left);
        r(1, 3) = -(top + bottom) / (top - bottom);
        r(2, 3) = -(far + near) / (far - near);
        return r;
    }
};