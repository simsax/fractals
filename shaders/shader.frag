#version 450 core

out vec4 fragColor;

uniform vec2 iResolution;
uniform float iTime;
uniform vec2 xCoords;
uniform vec2 yCoords;

// source: https://iquilezles.org/articles/palettes
vec3 palette(in float t)
{
    if (t == 1.0)
        return vec3(0.0);

    t = pow(t, 0.2);

    vec3 a = vec3(0.5, 0.5, 0.5);
    vec3 b = vec3(0.5, 0.5, 0.5);
    vec3 c = vec3(1.0, 1.0, 0.5);
    vec3 d = vec3(0.8, 0.9, 0.3);

    return a + b*cos( 6.28318*(c*t+d) );
}

float mandelbrot(float x0, float y0, float escape_radius, int max_iterations, bool smooth_) {

    float x = 0.0;
    float y = 0.0;
    float x2 = 0.0;
    float y2 = 0.0;

    for (int iterations = 0; iterations < max_iterations; iterations++) {
        y = 2.0 * x * y + y0;
        x = x2 - y2 + x0;
        x2 = x * x;
        y2 = y * y;

        if (x2 + y2 > escape_radius * escape_radius) {
            // mu = N + 1 - log (log  |Z(N)|) / log 2
            if (smooth_)
                return float(iterations) + 1.0 - log(log(float(sqrt(x2 + y2)))) / log(2.0);
            else
                return float(iterations);
        }
    }

    return max_iterations;
}


void main()
{
    vec2 uv = gl_FragCoord.xy / iResolution.x;

    float x0 = float(mix(xCoords.x, xCoords.y, uv.x));
    float y0 = float(mix(yCoords.x, yCoords.y, uv.y));

    int max_iterations = 100;
    bool smooth_ = true;
    float escape_radius = 10.0;

    float escape_count = mandelbrot(x0, y0, escape_radius, max_iterations, smooth_);
    float stability = clamp(escape_count / float(max_iterations), 0.0, 1.0);
    vec3 color = palette(stability);

    fragColor = vec4(color, 1.0);
}
