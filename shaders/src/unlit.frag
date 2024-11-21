#version 450

#include "glitch/fragment_base.glsl"
#include "glitch/vertex_base.glsl"

layout (location = 0) in vec3 v_position;
layout (location = 1) in vec2 v_normal;
layout (location = 2) in vec2 v_uv;

layout (location = 0) out vec4 o_color;

void main() { 
    o_color = texture(u_albedo_texture, v_uv)
                * u_material_data.base_color;
}
