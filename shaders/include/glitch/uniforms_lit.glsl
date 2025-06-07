#ifndef UNIFORMS_LIT_GLSL
#define UNIFORMS_LIT_GLSL

layout(set = 0, binding = 0) uniform MaterialData {
	vec4 base_color;
	float metallic;
	float roughness;
}
u_material_data;

layout(set = 0, binding = 1) uniform sampler2D u_albedo_texture;
layout(set = 0, binding = 2) uniform sampler2D u_normal_texture;

#endif // UNIFORMS_LIT_GLSL
