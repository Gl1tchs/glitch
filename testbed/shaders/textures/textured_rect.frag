#version 450

layout(location = 0) out vec4 o_color;

layout(location = 0) in vec2 v_uv;

layout(set = 0, binding = 0) uniform sampler2D u_texture;

void main() {
    o_color = texture(u_texture, v_uv);
}
