module lighting_pbr;

import light_sources;

static const float PI = 3.14159265359;

float distribution_GGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float geometry_schlick_GGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0; // k_direct

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float geometry_smith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggxV = geometry_schlick_GGX(NdotV, roughness);
    float ggxL = geometry_schlick_GGX(NdotL, roughness);

    return ggxV * ggxL;
}

float3 fresnel_schlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(saturate(1.0 - cosTheta), 5.0);
}

float3 calc_light_pbr_core(float3 L, float3 N, float3 V, float3 base_color, float metallic, float roughness, float3 light_color)
{
    float3 H = normalize(V + L); 
    float3 F0 = float3(0.04); 
    F0 = lerp(F0, base_color, metallic);

    // Cook-Torrance BRDF
    float NDF = distribution_GGX(N, H, roughness);
    float G = geometry_smith(N, V, L, roughness);
    float3 F = fresnel_schlick(max(dot(H, V), 0.0), F0);

    float3 kS = F;
    float3 kD = float3(1.0) - kS;
    kD *= (1.0 - metallic);

    float3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    float3 specular = numerator / denominator;

    float NdotL = max(dot(N, L), 0.0);
    float3 diffuse = kD * base_color / PI;

    return (diffuse + specular) * light_color * NdotL;
}

// --- Light Source Wrappers ---

public float3 calc_dir_light(DirectionalLight light, float3 N, float3 V, float3 base_color, float metallic, float roughness)
{
    float3 L = normalize(-light.direction.xyz);
    return calc_light_pbr_core(L, N, V, base_color, metallic, roughness, light.color.rgb);
}

public float3 calc_point_light(PointLight light, float3 world_pos, float3 N, float3 V, float3 base_color, float metallic, float roughness)
{
    float3 L_dir = light.position.xyz - world_pos;
    float distance = length(L_dir);
    float3 L = normalize(L_dir);

    float attenuation = 1.0 / (1.0 + light.linear * distance + light.quadratic * distance * distance);
    
    float3 radiance = calc_light_pbr_core(L, N, V, base_color, metallic, roughness, light.color.rgb);
    
    return radiance * attenuation;
}