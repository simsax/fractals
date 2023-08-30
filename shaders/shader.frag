#version 450 core

out vec4 fragColor;

uniform vec2 iResolution;
uniform float iTime;
uniform vec2 xCoords;
uniform vec2 yCoords;

// source: https://gist.github.com/yiwenl/745bfea7f04c456e0101
vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
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

    int max_iterations = 1000;
    bool smooth_ = true;
    float escape_radius = 10.0;

    float escape_count = mandelbrot(x0, y0, escape_radius, max_iterations, smooth_);
    float stability = clamp(escape_count / float(max_iterations), 0.0, 1.0);
    // float colorf = escape_count / float(max_iterations);
    // colorf = 1.0 - clamp(colorf, 0.0, 1.0);
    // vec3 color = vec3(colorf);
    vec3 hsv = vec3(pow(stability, 0.6), 1.0, 1.0 - stability);
    vec3 color = hsv2rgb(hsv);

    fragColor = vec4(color, 1.0);
}

// problems:
// - color depends on max_iterations
// - find if it's possible to increase precision without too much performance loss
// - check escape radius