#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <math.h>

typedef struct {
    float red;
    float green;
    float blue;
} RGBColor;

const int HEIGHT = 512;
const int WIDTH = 512;

RGBColor fragment(float u, float v)
{
    RGBColor color;
    int n = 20;
    color.red = (1 + sin(u * n)) * 0.5;
    color.green = (1 + cos(v * n)) * 0.5;
    color.blue = (1 + sin((u + v) * n)) * 0.5;

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

            RGBColor color = fragment(u, v);

            fputc((int)(color.red * 255), file);
            fputc((int)(color.green * 255), file);
            fputc((int)(color.blue * 255), file);
        }
    }

    fclose(file);

    return 0;
}