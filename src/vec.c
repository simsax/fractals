#include "vec.h"

Vec2 vec2(float x) { return (Vec2) { x, x }; }

Vec2 vec2_add(Vec2 a, Vec2 b) { return (Vec2) { a.x + b.x, a.y + b.y }; }

Vec2 vec2_sub(Vec2 a, Vec2 b) { return (Vec2) { a.x - b.x, a.y - b.y }; }

Vec2 vec2_mul(Vec2 a, Vec2 b) { return (Vec2) { a.x * b.x, a.y * b.y }; }

Vec2 vec2_div(Vec2 a, Vec2 b) { return (Vec2) { a.x / b.x, a.y / b.y }; }

float vec2_dot(Vec2 a, Vec2 b) { return a.x * b.x + a.y * b.y; }

float vec2_length(Vec2 a) { return sqrt(a.x * a.x + a.y * a.y); }