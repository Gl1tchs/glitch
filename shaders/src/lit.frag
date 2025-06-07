#version 450

#include "glitch/mesh_definition.glsl"
#include "glitch/uniforms_lit.glsl"

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_uv;

layout(location = 0) out vec4 o_color;

// Simple light params
const vec3 light_pos = vec3(10000.0, 10000.0, 10000.0);
const vec3 light_color = vec3(0.98, 1.0, 0.18);

const float ambient_strength = 0.1;
const float specular_strength = 0.5;
const float shininess = 32.0;

void main() {
	// Material
	vec4 albedo_tex = texture(u_albedo_texture, v_uv);
	vec3 base_color = u_material_data.base_color.rgb * albedo_tex.rgb;

	// Load normal from normal map
	vec3 normal_map = texture(u_normal_texture, v_uv).rgb;
	// Transform from [0,1] → [-1,1]
	normal_map = normalize(normal_map * 2.0 - 1.0);

	// For now assume object space normal map (no tangent space):
	vec3 norm = normalize(normal_map);

	// Light direction
	vec3 light_dir = normalize(light_pos - v_position);

	// View direction
	vec3 view_dir = normalize(
			u_push_constants.scene_buffer.camera_position - v_position);

	// Ambient
	vec3 ambient = ambient_strength * base_color;

	// Diffuse
	float diff = max(dot(norm, light_dir), 0.0);
	vec3 diffuse = diff * base_color * light_color;

	// Specular
	vec3 reflect_dir = reflect(-light_dir, norm);
	float spec = pow(max(dot(view_dir, reflect_dir), 0.0), shininess);
	vec3 specular = specular_strength * spec * light_color;

	// Final color
	vec3 result = ambient + diffuse + specular;
	o_color = vec4(result, u_material_data.base_color.a);
}
