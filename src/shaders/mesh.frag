#version 450

#include "input_structures.glsl"
#include "sky_light.glsl"

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_uv;

layout(location = 0) out vec4 o_color;

void main() {
    SkyLight sun;
    sun.direction = scene_data.sun_direction.xyz;
    sun.color = scene_data.sun_color.rgb;

    Material material;
    material.diffuse = texture(color_tex, v_uv).rgb;
    material.shininess = 1.0f;

    vec3 norm = normalize(v_normal);
    vec3 view_dir = normalize(scene_data.camera_pos.xyz - v_position);

    vec3 lighing_result = calc_sky_light(sun, material, norm, view_dir);

    o_color = vec4(lighing_result, 1.0f);
}
