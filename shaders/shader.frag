#version 450 core

out vec4 fragColor;

uniform vec2 iResolution;
uniform float iTime;


void main()
{
    vec2 uv = gl_FragCoord.xy / iResolution.x;

    float x0 = mix(-2.00, 0.47, uv.x);
    float y0 = mix(-1.12, 1.12, uv.y);

    int iterations = 0;
    int max_iterations = 100;

    vec2 z = vec2(0.0);

    for (; iterations < max_iterations && z.x * z.x + z.y * z.y <= 4; iterations++) {
        float x_temp = z.x * z.x - z.y * z.y + x0;
        z.y = 2.0 * z.x * z.y + y0;
        z.x = x_temp;
    }

    // float colorf = 1 - float(iterations) / float(max_iterations);
    float colorf = float(iterations) / float(max_iterations);

    vec3 color = vec3(colorf);

    fragColor = vec4(color, 1.0);
}
