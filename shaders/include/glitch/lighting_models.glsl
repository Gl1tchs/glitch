#ifndef LIGHTING_MODELS_GLSL
#define LIGHTING_MODELS_GLSL

#define GLITCH_USE_PBR 1

#if GLITCH_USE_PBR
#include "glitch/pbr_lighting.glsl"
#else
#include "glitch/phong_lighting.glsl"
#endif

// Atten = 1.0 / (1.0 + linear * distance + quadratic * distance^2)
float calculate_attenuation(float distance, float linear, float quadratic) {
    return 1.0 / (1.0 + linear * distance + quadratic * distance * distance);
}

vec3 calc_dir_light(DirectionalLight light, vec3 N, vec3 V, vec3 base_color,
    float metallic, float roughness) {
    vec3 L = normalize(-light.direction.xyz);

    #if GLITCH_USE_PBR
    return calc_light_pbr(
        L, N, V, base_color, metallic, roughness, light.color.rgb);
    #else
    return calc_light_blinn_phong(L, N, V, light.color.rgb, base_color);
    #endif
}

vec3 calc_point_light(PointLight light, vec3 P, vec3 N, vec3 V, vec3 base_color,
    float metallic, float roughness) {
    vec3 light_vec = light.position.xyz - P;
    float distance = length(light_vec);
    vec3 L = normalize(light_vec);

    float attenuation =
        calculate_attenuation(distance, light.linear, light.quadratic);

    #if GLITCH_USE_PBR
    vec3 light_contribution = calc_light_pbr(
            L, N, V, base_color, metallic, roughness, light.color.rgb);
    #else
    vec3 light_contribution =
        calc_light_blinn_phong(L, N, V, light.color.rgb, base_color);
    #endif

    return light_contribution * attenuation;
}

#endif // LIGHTING_MODELS_GLSL
