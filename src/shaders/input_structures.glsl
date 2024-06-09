#ifndef INPUT_STRUCTURES_GLSL
#define INPUT_STRUCTURES_GLSL

layout(set = 0, binding = 0) uniform SceneData {
    vec4 camera_pos;
    mat4 view;
    mat4 proj;
    mat4 viewproj;
    vec4 sun_direction; // w for power
    vec4 sun_color;
} scene_data;

layout(set = 1, binding = 0) uniform MaterialData {
    vec4 color_factor;
    float metallic_factor;
    float roughness_factor;
} material_data;

layout(set = 1, binding = 1) uniform sampler2D color_texture;
layout(set = 1, binding = 2) uniform sampler2D roughness_texture;
layout(set = 1, binding = 3) uniform sampler2D normal_texture;

#endif
