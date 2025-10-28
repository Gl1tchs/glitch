#version 450
#extension GL_EXT_buffer_reference : require

#include "glitch/common.glsl"
#include "glitch/lighting_models.glsl"

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_uv;

layout(location = 0) out vec4 o_color;

void main() {
    // TODO: Normal mappig
    const vec3 N = normalize(v_normal);
    const vec3 V = normalize(
            u_push_constants.scene_buffer.camera_position.xyz - v_position);

    const vec4 albedo_sample = texture(u_diffuse_texture, v_uv);
    const vec3 base_color = albedo_sample.rgb *
            u_material_data.base_color.rgb; // Added material base color tint

    // Standard PBR packing: R=Ambient Occlusion, G=Roughness, B=Metallic
    const vec4 mrao_sample = texture(u_metallic_roughness_texture, v_uv);

    // Sometimes AO is in its own texture. We'll use the separate one.
    // TODO: maybe not provided
    const float ao = texture(u_ambient_occlusion_texture, v_uv).r;

    const float roughness = mrao_sample.g * u_material_data.roughness;
    const float metallic = mrao_sample.b * u_material_data.metallic;

    vec3 lighting_color = vec3(0.0);

    // Directional Light
    lighting_color +=
        calc_dir_light(u_push_constants.scene_buffer.directional_light, N,
            V, base_color, metallic, roughness);

    // Point Lights
    for (int i = 0; i < u_push_constants.scene_buffer.num_point_lights; ++i) {
        lighting_color +=
            calc_point_light(u_push_constants.scene_buffer.point_lights[i],
                v_position, N, V, base_color, metallic, roughness);
    }

    // In a real PBR setup, this would be Image-Based Lighting (IBL).
    const vec3 ambient = vec3(0.05) * base_color * ao;

    // Ambient added *after* direct lighting, as AO only affects it
    vec3 final_color = ambient + lighting_color;

    // Simple gamma correction
    final_color = pow(final_color, vec3(1.0 / 2.2));

    o_color = vec4(final_color, albedo_sample.a);
}
