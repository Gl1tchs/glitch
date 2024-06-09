#ifndef MATERIAL_GLSL
#define MATERIAL_GLSL

struct Material {
    vec4 color_factor;
    float metallic_factor;
    float roughness_factor;
    vec3 color_image;
    vec3 roughness_image;
    vec3 normal_image;
};

#endif
