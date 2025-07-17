#version 450

layout(location = 0) in vec3 v_world_pos;

layout(location = 0) out vec4 frag_color;

layout(push_constant) uniform PushConstants {
	mat4 view_proj;
	vec3 camera_pos;
	float grid_size;
}
pc;

float min_pixel_spacing = 2.0;
float cell_size = 0.025;
vec4 color_thin = vec4(0.5, 0.5, 0.5, 1.0);
vec4 color_thick = vec4(0.0, 0.0, 0.0, 1.0);

float log10(float x) { return log(x) / log(10.0); }

float max2(vec2 v) { return max(v.x, v.y); }

void main() {
	vec2 dx = vec2(dFdx(v_world_pos.x), dFdy(v_world_pos.x));
	vec2 dy = vec2(dFdx(v_world_pos.z), dFdy(v_world_pos.z));
	vec2 dudv = vec2(length(dx), length(dy)) * 4.0;

	float lod =
			max(0.0, log10(length(dudv) * min_pixel_spacing / cell_size) + 1.0);

	float cell0 = cell_size * pow(10.0, floor(lod));
	float cell1 = cell0 * 10.0;
	float cell2 = cell1 * 10.0;

	vec2 mod0 = mod(v_world_pos.xz, cell0) / dudv;
	float lod0a = max2(1.0 - abs(clamp(mod0 * 2.0 - 1.0, 0.0, 1.0)));

	vec2 mod1 = mod(v_world_pos.xz, cell1) / dudv;
	float lod1a = max2(1.0 - abs(clamp(mod1 * 2.0 - 1.0, 0.0, 1.0)));

	vec2 mod2 = mod(v_world_pos.xz, cell2) / dudv;
	float lod2a = max2(1.0 - abs(clamp(mod2 * 2.0 - 1.0, 0.0, 1.0)));

	float lod_frac = fract(lod);
	vec4 color;

	if (lod2a > 0.0) {
		color = color_thick * lod2a;
	} else if (lod1a > 0.0) {
		color = mix(color_thick, color_thin, lod_frac) * lod1a;
	} else {
		color = color_thin * (lod0a * (1.0 - lod_frac));
	}

	float fade = 1.0 -
			clamp(length(v_world_pos.xz - pc.camera_pos.xz) / pc.grid_size, 0.0,
					1.0);
	color.a *= fade;

	frag_color = color;
}
