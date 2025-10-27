#ifndef PHONG_LIGHTING_GLSL
#define PHONG_LIGHTING_GLSL

// L_out = C_L * [ C_D * max(0, N . L) + C_S * max(0, N . H)^Shininess ]
// where H = normalize(L + V) and C_S = vec3(1.0)
vec3 calc_light_blinn_phong(
		vec3 L, vec3 N, vec3 V, vec3 light_color, vec3 material_base_color) {
	float diff_term = max(dot(N, L), 0.0);
	vec3 diffuse = material_base_color * light_color * diff_term;

	// Specular component using Halfway vector
	vec3 H = normalize(L + V);
	float spec_term = pow(max(dot(N, H), 0.0), MAT_SHININESS);
	vec3 specular = vec3(1.0) * light_color * spec_term;

	return diffuse + specular;
}

#endif // PHONG_LIGHTING_GLSL