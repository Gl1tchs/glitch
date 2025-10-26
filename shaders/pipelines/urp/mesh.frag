#version 450

#include "glitch/common.glsl"

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_uv;

layout(location = 0) out vec4 o_color;

// Simple light params
const vec3 light_pos = vec3(-15.0, 12.5, -12.5);

const float ambient_strength = 0.1;
const float specular_strength = 0.5;
const float shininess = 32.0;

void main() {
	SceneBuffer scene_data = u_push_constants.scene_buffer;

	const vec3 light_color = scene_data.directional_light.color.rgb;

	// Sample textures
	vec3 diffuse_color = vec3(texture(u_diffuse_texture, v_uv));
	vec3 specular_map = vec3(texture(u_metallic_roughness_texture, v_uv));
	float ao = texture(u_ambient_occlusion_texture, v_uv)
					   .r; // AO usually stored in R channel

	// Ambient
	vec3 ambient = ambient_strength * light_color * diffuse_color * ao;

	// Diffuse
	vec3 norm = normalize(v_normal); // Note: If you want, you can do normal
									 // mapping here using u_normal_texture
	vec3 light_dir = normalize(light_pos - v_position);
	// vec3 light_dir = scene_data.directional_light.direction.xyz;
	float diff = max(dot(norm, light_dir), 0.0);

	vec3 diffuse = diff * light_color * diffuse_color * ao;

	// Specular
	vec3 view_dir = normalize(scene_data.camera_position - v_position);
	vec3 reflect_dir = reflect(-light_dir, norm);
	float spec = pow(max(dot(view_dir, reflect_dir), 0.0), shininess);

	vec3 specular = specular_strength * spec * specular_map * light_color * ao;

	// Final color
	vec3 result =
			(ambient + diffuse + specular) * u_material_data.base_color.rgb;

	o_color = vec4(result, 1.0);
}
