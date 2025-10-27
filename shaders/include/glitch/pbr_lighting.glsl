#ifndef PBR_LIGHTING_GLSL
#define PBR_LIGHTING_GLSL

const float PI = 3.14159265359;

/**
 * (D) Normal Distribution Function (GGX Trowbridge-Reitz)
 * Estimates the amount of microfacets aligned with the halfway vector (H)
 */
float distribution_GGX(vec3 N, vec3 H, float roughness) {
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float num = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return num / denom;
}

/**
 * (G) Geometry Function (Schlick-GGX)
 * Describes the self-shadowing properties of the microfacets
 */
float geometry_schlick_GGX(float NdotV, float roughness) {
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0; // k_direct

	float num = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return num / denom;
}

/**
 * (G) Geometry Function (Smith's method)
 * Combines geometry for both view (V) and light (L) directions
 */
float geometry_smith(vec3 N, vec3 V, vec3 L, float roughness) {
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggxV = geometry_schlick_GGX(NdotV, roughness);
	float ggxL = geometry_schlick_GGX(NdotL, roughness);

	return ggxV * ggxL;
}

/**
 * (F) Fresnel Equation (Schlick's approximation)
 * Describes the ratio of light that is reflected vs. refracted
 */
vec3 fresnel_schlick(float cosTheta, vec3 F0) {
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

/**
 * PBR Cook-Torrance BRDF calculation for a single light.
 */
vec3 calc_light_pbr(vec3 L, vec3 N, vec3 V, vec3 base_color, float metallic,
		float roughness, vec3 light_color) {
	vec3 H = normalize(V + L); // Halfway vector

	// F0 = Base reflectivity. 0.04 for non-metals, base_color for metals.
	vec3 F0 = vec3(0.04);
	F0 = mix(F0, base_color, metallic);

	// Cook-Torrance BRDF terms
	float NDF = distribution_GGX(N, H, roughness);
	float G = geometry_smith(N, V, L, roughness);
	vec3 F = fresnel_schlick(max(dot(H, V), 0.0), F0);

	// Specular BRDF
	vec3 kS = F; // Specular ratio
	vec3 kD = vec3(1.0) - kS; // Diffuse ratio
	kD *= (1.0 - metallic); // Metals have no diffuse light

	vec3 numerator = NDF * G * F;
	float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) +
			0.0001; // 0.0001 to prevent div by zero
	vec3 specular = numerator / denominator;

	// Combine diffuse and specular
	// NdotL is the common factor for both, representing incoming light
	float NdotL = max(dot(N, L), 0.0);

	// (kD * base_color / PI) is the diffuse BRDF (Lambertian)
	vec3 diffuse = kD * base_color / PI;

	// Final outgoing radiance
	return (diffuse + specular) * light_color * NdotL;
}

#endif // PBR_LIGHTING_GLSL