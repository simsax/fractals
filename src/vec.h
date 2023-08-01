#ifndef VEC_H
#define VEC_H

#include <math.h>

typedef struct {
    float x;
    float y;
} Vec2;

Vec2 vec2(float x);
Vec2 vec2_add(Vec2 a, Vec2 b);
Vec2 vec2_sub(Vec2 a, Vec2 b);
Vec2 vec2_mul(Vec2 a, Vec2 b);
Vec2 vec2_div(Vec2 a, Vec2 b);
float vec2_dot(Vec2 a, Vec2 b);
float vec2_length(Vec2 a);

#endif // VEC_H
