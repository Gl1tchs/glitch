module common;

import light_sources;

public struct MeshVertex
{
    public float3 position;
    public float uv_x;
    public float3 normal;
    public float uv_y;

    public property float2 uv 
    { 
        get { return float2(uv_x, uv_y); } 
    }
};

public struct SceneBuffer
{
    public float4x4 view_projection;
    public float4 camera_position;

    public int num_point_lights;
    private int3 __pad0;

    public DirectionalLight directional_light;
    public PointLight point_lights[16];
};

public struct PushConstants
{
    public float4x4 transform;
    public MeshVertex* vertex_buffer; // Pointer to the buffer
    public SceneBuffer* scene_buffer; // Pointer to the buffer
};

[[vk::push_constant]]
public ConstantBuffer<PushConstants> u_push_constants;
