#ifndef MATERIAL_DATA_GLSL
#define MATERIAL_DATA_GLSL

layout (set = 1, binding = 0) uniform MaterialData {
    vec4 base_color;
    float metallic;
    float roughness;
} u_material_data;

layout (set = 1, binding = 1) uniform sampler2D u_albedo_texture;
layout (set = 1, binding = 2) uniform sampler2D u_normal_texture;

#endif // MATERIAL_DATA_GLSL
