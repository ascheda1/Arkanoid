#pragma once
#include <SDL.h>
#include <cmath>

inline float clampf(float v, float a, float b) { return (v < a) ? a : (v > b) ? b
                                                                              : v; }

struct vec2
{
    float x, y;
    vec2() : x(0), y(0) {}
    vec2(float _x, float _y) : x(_x), y(_y) {}
    vec2 operator+(const vec2 &o) const { return vec2(x + o.x, y + o.y); }
    vec2 operator-(const vec2 &o) const { return vec2(x - o.x, y - o.y); }
    vec2 operator*(float s) const { return vec2(x * s, y * s); }
};

inline float dot(const vec2 &a, const vec2 &b) { return a.x * b.x + a.y * b.y; }
inline float length(const vec2 &v) { return std::sqrt(v.x * v.x + v.y * v.y); }
inline vec2 normalize(const vec2 &v)
{
    float l = length(v);
    if (l == 0)
        return vec2(0, 0);
    return vec2(v.x / l, v.y / l);
}

inline vec2 reflect(const vec2 &v, const vec2 &n)
{
    // assume n normalized
    float d = dot(v, n);
    return vec2(v.x - 2 * d * n.x, v.y - 2 * d * n.y);
}