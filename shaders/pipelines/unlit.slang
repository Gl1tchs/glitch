import common;

public struct MaterialData
{
    public float4 base_color;
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

[shader("vertex")]
VertexStageOutput vertexMain(uint vertexID : SV_VertexID)
{
    VertexStageOutput output;

    MeshVertex v = u_push_constants.vertex_buffer[vertexID];

    SceneBuffer scene_data = *u_push_constants.scene_buffer;

    float4 frag_pos = mul(u_push_constants.transform, float4(v.position, 1.0));

    output.sv_position = mul(scene_data.view_projection, frag_pos);

    output.position = frag_pos.xyz;
    output.normal = v.normal;
    
    output.uv = v.uv; 

    return output;
}

[shader("fragment")]
float4 fragmentMain(VertexStageOutput input) : SV_Target
{
    float4 color = u_diffuse_texture.Sample(input.uv);
    
    return color * u_material_data.base_color;
}