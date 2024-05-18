#version 450

layout(location = 0) in vec3 in_color;
layout(location = 1) in vec2 in_uv;

// output write
layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 0) uniform sampler2D color_texture;

void main() {
    out_color = vec4(texture(color_texture, in_uv).xyz, 1.0f);
}
