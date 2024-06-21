#version 450
#extension GL_OES_standard_derivatives : enable

precision highp float;

#include "input_structures.glsl"

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_uv;

layout(location = 0) out vec4 o_color;

const float AMBIENT_STRENGTH = 0.1f;

vec3 get_dir_light(vec3 p_normal, vec3 p_view_dir) {
    vec3 ambient = AMBIENT_STRENGTH * scene_data.sun_color.rgb
            * material_data.diffuse_factor.rgb
            * texture(diffuse_texture, v_uv).rgb;

    vec3 light_dir = normalize(-scene_data.sun_direction.xyz);

    float diff = max(dot(p_normal, light_dir), 0.0);
    vec3 diffuse = scene_data.sun_color.rgb * diff
            * material_data.diffuse_factor.rgb
            * texture(diffuse_texture, v_uv).rgb;

    // vec3 reflect_dir = reflect(-light_dir, p_normal);

    // Blinn-Phong
    vec3 halfway_dir = normalize(light_dir + p_view_dir);

    float spec = pow(max(dot(p_view_dir, halfway_dir), 0.0), material_data.shininess);
    vec3 specular = scene_data.sun_color.rgb * spec
            * texture(specular_texture, v_uv).rgb;

    return ambient + diffuse + specular;
}

void main() {
    mat3 normal_mat = transpose(inverse(mat3(push_constants.transform)));
    vec3 normal = normalize(normal_mat * v_normal);

    vec3 view_dir = normalize(scene_data.camera_pos.xyz - v_position);

    vec3 light_result = get_dir_light(normal, view_dir);

    o_color = vec4(light_result, 1.0);
}
