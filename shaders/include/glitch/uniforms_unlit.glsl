#ifndef UNIFORMS_UNLIT_GLSL
#define UNIFORMS_UNLIT_GLSL

layout(set = 0, binding = 0) uniform MaterialData {
	vec4 base_color;
	float metallic;
	float roughness;
}
u_material_data;

layout(set = 0, binding = 1) uniform sampler2D u_albedo_texture;

#endif // UNIFORMS_UNLIT_GLSL
