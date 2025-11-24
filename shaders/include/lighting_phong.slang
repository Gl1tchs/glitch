module lighting_phong;

static const float MAT_SHININESS = 32.0;

public float3 calc_light_blinn_phong(
    float3 L, 
    float3 N, 
    float3 V, 
    float3 light_color, 
    float3 material_base_color) 
{
    float diff_term = max(dot(N, L), 0.0);
    float3 diffuse = material_base_color * light_color * diff_term;

    // Specular component using Halfway vector
    float3 H = normalize(L + V);
    float spec_term = pow(max(dot(N, H), 0.0), MAT_SHININESS);
    float3 specular = float3(1.0) * light_color * spec_term;

    return diffuse + specular;
}