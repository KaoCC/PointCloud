#pragma once

#include <cmath>

struct Vertex {
    float x = 0;
    float y = 0;
    float z = 0;
    float r = 1.0f;
};

struct Triangle {
    int v0, v1, v2;
};

struct Vec {
    float x = 0;
    float y = 0;
    float z = 0;
    float w = 1.0f;
};

inline Vec operator+(const Vec &a, const Vec &b) { return Vec{a.x + b.x, a.y + b.y, a.z + b.z}; }
inline Vec operator*(float f, const Vec &b) { return Vec{f * b.x, f * b.y, f * b.z}; }
inline Vec operator*(const Vec &a, float f) { return Vec{a.x * f, a.y * f, a.z * f}; }
inline float dot(const Vec &a, const Vec &b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline Vec normalize(const Vec &a) { return a * (1.0 / sqrt(dot(a, a))); }
