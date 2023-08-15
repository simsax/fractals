#version 450 core

out vec4 fragColor;

uniform vec2 iResolution;
uniform float iTime;

void main()
{
    vec2 uv = gl_FragCoord.xy / iResolution * 2 - 1;
    // fragColor = vec4(sin(uv.x * 10 * iTime), cos(uv.y * 10 * iTime), 0.0f, 1.0f);
    float len = sqrt(uv.x * uv.x + uv.y * uv.y) - 0;
    fragColor = vec4(0.0f, 0.0f, len, 1.0f);
    // fragColor = vec4(0.0f, len, 0.0f, 1.0f);
    // }a
}
