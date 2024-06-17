#version 450

#include "input_structures.glsl"
#include "pbr.glsl"

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_uv;

layout(location = 0) out vec4 o_color;

void main() {
    vec3 N = get_normal(v_position, scene_data.camera_pos.xyz, v_normal,
            texture(normal_texture, v_uv).rgb, v_uv);
    vec3 V = normalize(scene_data.camera_pos.xyz - v_position);
    vec3 L = normalize(-scene_data.sun_direction.xyz);
    vec3 H = normalize(V + L);

    vec3 base_color = mix(material_data.color_factor.rgb,
            texture(color_texture, v_uv).rgb,
            material_data.color_factor.a);

    float metallic = material_data.metallic_factor;

    float roughness = mix(material_data.roughness_factor,
            texture(roughness_texture, v_uv).r,
            material_data.roughness_factor);

    // Calculate reflectance at normal incidence
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, base_color, metallic);

    // Calculate the Fresnel effect
    vec3 F = fresnel_schlick(max(dot(H, V), 0.0), F0);

    // Calculate normal distribution function
    float D = distribution_ggx(N, H, roughness);

    // Calculate geometry function
    float G = geometry_smith(N, V, L, roughness);

    // Calculate specular term
    vec3 numerator = D * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0)
            + EPSILON;

    vec3 specular = numerator / denominator;

    // Calculate the kS and kD terms
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    // Calculate final color
    float NdotL = max(dot(N, L), 0.0);
    vec3 Lo = (kD * base_color / PI + specular) * scene_data.sun_color.rgb
            * NdotL;

    // Ambient lighting (assuming a simple ambient factor)
    vec3 ambient = vec3(0.03) * base_color;
    vec3 color = ambient + Lo;

    // Gamma correction
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    o_color = vec4(color, 1.0f);
}
