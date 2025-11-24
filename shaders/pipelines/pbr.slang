import glsl;
import common;
import lighting_pbr;

public struct MaterialData
{
    public float4 base_color;
    public float metallic;
    public float roughness;
};

public struct VertexStageOutput
{
    public float4 sv_position : SV_Position; // System Value: Position
    public float3 position;                  // Location 0 (automatic)
    public float3 normal;                    // Location 1
    public float2 uv;                        // Location 2
};

[[vk::binding(0, 0)]] 
public ConstantBuffer<MaterialData> u_material_data;

[[vk::binding(1, 0)]] public Sampler2D u_diffuse_texture;
[[vk::binding(2, 0)]] public Sampler2D u_normal_texture;
[[vk::binding(3, 0)]] public Sampler2D u_metallic_roughness_texture;
[[vk::binding(4, 0)]] public Sampler2D u_ambient_occlusion_texture;

[shader("vertex")]
VertexStageOutput vertexMain(uint vertexID : SV_VertexID)
{
    VertexStageOutput output;

    MeshVertex v = u_push_constants.vertex_buffer[vertexID];
    SceneBuffer scene_data = *u_push_constants.scene_buffer;

    float4 frag_pos = mul(u_push_constants.transform, float4(v.position, 1.0));
    output.sv_position = mul(scene_data.view_projection, frag_pos);

    output.position = frag_pos.xyz;

    // Normal Matrix calculation
    // Note: Calculating inverse() per vertex is expensive. 
    // Ideally pass a normal matrix in PushConstants.
    // Slang supports standard matrix intrinsics.
    float3x3 normalMatrix = (float3x3)transpose(inverse(u_push_constants.transform));
    output.normal = mul(normalMatrix, v.normal);

    output.uv = v.uv;

    return output;
}

[shader("fragment")]
float4 fragmentMain(VertexStageOutput input) : SV_Target
{
    float3 N = normalize(input.normal);
    SceneBuffer scene_data = *u_push_constants.scene_buffer;
    float3 V = normalize(scene_data.camera_position.xyz - input.position);

    float4 albedo_sample = u_diffuse_texture.Sample(input.uv);
    float3 base_color = albedo_sample.rgb * u_material_data.base_color.rgb;

    float4 mrao_sample = u_metallic_roughness_texture.Sample(input.uv);
    
    float ao = u_ambient_occlusion_texture.Sample(input.uv).r;

    float roughness = mrao_sample.g * u_material_data.roughness;
    float metallic = mrao_sample.b * u_material_data.metallic;

    float3 lighting_color = float3(0.0);

    lighting_color += calc_dir_light(
        scene_data.directional_light, 
        N, V, base_color, metallic, roughness
    );

    for (int i = 0; i < scene_data.num_point_lights; ++i)
    {
        lighting_color += calc_point_light(
            scene_data.point_lights[i], 
            input.position, 
            N, V, base_color, metallic, roughness
        );
    }

    float3 ambient = float3(0.05) * base_color * ao;
    float3 final_color = ambient + lighting_color;

    final_color = pow(final_color, float3(1.0 / 2.2));

    return float4(final_color, albedo_sample.a);
}