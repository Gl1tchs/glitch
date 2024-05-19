#version 450

#include "input_structures.glsl"

layout(location = 0) in vec3 in_color;
layout(location = 1) in vec2 in_uv;

// output write
layout(location = 0) out vec4 out_color;

void main() {
    out_color = vec4(texture(color_tex, in_uv).xyz, 1.0f);
}
