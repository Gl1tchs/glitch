#ifndef INPUT_STRUCTURES_GLSL
#define INPUT_STRUCTURES_GLSL

layout(set = 0, binding = 0) uniform MaterialConstants {
    vec4 color_factors;
    vec4 metal_rough_factors;
} material_data;

layout(set = 0, binding = 1) uniform sampler2D color_tex;
layout(set = 0, binding = 2) uniform sampler2D metal_rough_tex;

#endif

