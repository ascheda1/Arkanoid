#pragma once
#include <SDL.h>
#include <cmath>

inline float clampf(float v, float a, float b) { return (v < a) ? a : (v > b) ? b : v; }

struct Vec2 {
    float x, y;
    Vec2(): x(0), y(0) {}
    Vec2(float _x, float _y): x(_x), y(_y) {}
    Vec2 operator+(const Vec2& o) const { return Vec2(x+o.x,y+o.y); }
    Vec2 operator-(const Vec2& o) const { return Vec2(x-o.x,y-o.y); }
    Vec2 operator*(float s) const { return Vec2(x*s, y*s); }
};

inline float dot(const Vec2 &a, const Vec2 &b) { return a.x*b.x + a.y*b.y; }
inline float length(const Vec2 &v) { return std::sqrt(v.x*v.x + v.y*v.y); }
inline Vec2 normalize(const Vec2 &v) {
    float l = length(v);
    if (l == 0) return Vec2(0,0);
    return Vec2(v.x / l, v.y / l);
}

inline Vec2 reflect(const Vec2 &v, const Vec2 &n) {
    // assume n normalized
    float d = dot(v, n);
    return Vec2(v.x - 2*d*n.x, v.y - 2*d*n.y);
}
