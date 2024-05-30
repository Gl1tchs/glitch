#version 450

#include "input_structures.glsl"

layout(location = 0) in vec4 in_position;
layout(location = 1) in vec4 in_color;
layout(location = 2) in vec2 in_uv;

layout(location = 0) out vec4 out_color;
layout(location = 1) out vec4 out_position;

void main() {
    out_color = vec4(texture(color_tex, in_uv).xyz, 1.0f);

    out_position = in_position;
}
