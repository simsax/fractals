#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include "vec.h"

typedef struct {
    float red;
    float green;
    float blue;
} RGBColor;

const int HEIGHT = 512;
const int WIDTH = 512;

RGBColor stripes(Vec2 uv)
{
    RGBColor color;
    int n = 20;
    color.red = (1 + sin(uv.x * n)) * 0.5;
    color.green = (1 + cos(uv.y * n)) * 0.5;
    color.blue = (1 + sin((uv.x + uv.y) * n)) * 0.5;

    return color;
}

RGBColor circle(Vec2 uv)
{
    RGBColor color;
    Vec2 center = { 0.5f, 0.5f };
    float radius = 0.2;
    Vec2 diff = vec2_sub(uv, center);
    int res = vec2_length(diff) > radius;
    color = (RGBColor) { 1, (float)res, (float)res };

    return color;
}

RGBColor circle_gradient(Vec2 uv)
{
    RGBColor color;
    Vec2 center = { 0.5f, 0.5f };
    float radius = 0.2;
    Vec2 diff = vec2_sub(uv, center);
    float res = 1 - fmin(vec2_length(diff) / radius, 1);
    color = (RGBColor) { res, 0, 0 };

    return color;
}

int main(void)
{
    const char* filename = "output.ppm";
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Failed at opening file %s", filename);
        exit(1);
    }
    fprintf(file, "P6 %d %d %d\n", WIDTH, HEIGHT, 255);

    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            float u = (float)i / HEIGHT;
            float v = (float)j / WIDTH;

            RGBColor color = circle_gradient((Vec2) { u, v });

            fprintf(file, "%c%c%c", (char)(color.red * 255), (char)(color.green * 255),
                (char)(color.blue * 255));
        }
    }

    fclose(file);

    return 0;
}