#version 450
#extension GL_OES_standard_derivatives : enable

precision highp float;

#include "input_structures.glsl"

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_uv;

layout(location = 0) out vec4 o_color;

const float AMBIENT_CONSTANT = 0.15f;

vec3 get_dir_light(vec3 p_normal, vec3 p_view_dir) {
    vec3 light_dir = normalize(-scene_data.sun_direction.xyz);
    vec3 halfway_dir = normalize(light_dir + p_view_dir);

    vec3 color = texture(diffuse_texture, v_uv).rgb
            * material_data.diffuse_factor.rgb;

    float diff = max(dot(light_dir, p_normal), 0.0);
    float spec = pow(max(dot(p_normal, halfway_dir), 0.0),
            material_data.shininess_factor);

    vec3 ambient = AMBIENT_CONSTANT * scene_data.sun_color.rgb;
    vec3 diffuse = scene_data.sun_color.rgb * diff;
    vec3 specular = scene_data.sun_color.rgb * spec
            * (material_data.metallic_factor / 2.0f);

    return (ambient + (diffuse + specular)) * color;
}

void main() {
    mat3 normal_mat = transpose(inverse(mat3(push_constants.transform)));
    vec3 normal = normalize(normal_mat * v_normal);

    vec3 view_dir = normalize(scene_data.camera_pos.xyz - v_position);

    vec3 light_result = get_dir_light(normal, view_dir);

    o_color = vec4(light_result, 1.0);
}
