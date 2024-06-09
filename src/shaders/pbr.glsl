#ifndef PBR_GLSL
#define PBR_GLSL

const float PI = 3.14159265359;
const float epsilon = 0.0001;

vec3 get_normal(vec3 p_frag_pos, vec3 p_view_pos, vec3 p_normal,
    vec3 p_normal_image, vec2 p_uv) {
    vec3 tangent_normal = p_normal_image * 2.0 - 1.0;

    vec3 Q1 = dFdx(p_frag_pos);
    vec3 Q2 = dFdy(p_frag_pos);

    vec2 st1 = dFdx(p_uv);
    vec2 st2 = dFdy(p_uv);

    vec3 N = normalize(p_normal);
    vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B = -normalize(cross(N, T));

    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangent_normal);
}

vec3 fresnel_schlick(float p_cos_theta, vec3 p_F0) {
    return p_F0 + (1.0 - p_F0) * pow(1.0 - p_cos_theta, 5.0);
}

float distribution_ggx(vec3 p_N, vec3 p_H, float p_roughness) {
    float a = p_roughness * p_roughness;
    float a2 = a * a;
    float NdotH = max(dot(p_N, p_H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float geometry_schlick_ggx(float p_NdotV, float p_roughness) {
    float r = (p_roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = p_NdotV;
    float denom = p_NdotV * (1.0 - k) + k;

    return nom / denom;
}

float geometry_smith(vec3 p_N, vec3 p_V, vec3 p_L, float p_roughness) {
    float NdotV = max(dot(p_N, p_V), 0.0);
    float NdotL = max(dot(p_N, p_L), 0.0);
    float ggx2 = geometry_schlick_ggx(NdotV, p_roughness);
    float ggx1 = geometry_schlick_ggx(NdotL, p_roughness);

    return ggx1 * ggx2;
}

#endif
