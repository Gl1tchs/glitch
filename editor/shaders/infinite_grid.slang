public struct PushConstants
{
    float4x4 view_proj;
    float3 camera_pos;
    float grid_size;
};

[[vk::push_constant]]
public ConstantBuffer<PushConstants> pc;

struct VertexOutput {
    float3 v_world_pos : TEXCOORD0; // location = 0
};

static const float3 positions[4] = {
    float3(-1.0, 0.0, -1.0),
    float3( 1.0, 0.0, -1.0),
    float3( 1.0, 0.0,  1.0),
    float3(-1.0, 0.0,  1.0) 
};

static const int indices[6] = {0, 2, 1, 2, 0, 3};

[shader("vertex")]
VertexOutput vertexMain(uint vertexID : SV_VertexID) {
    VertexOutput output;

    int idx = indices[vertexID];
    float3 pos = positions[idx] * pc.grid_size;

    pos.x += pc.camera_pos.x;
    pos.z += pc.camera_pos.z;

    output.v_world_pos = pos;
    
    return output;
}

static const float min_pixel_spacing = 2.0;
static const float cell_size = 0.025;
static const float4 color_thin = float4(0.5, 0.5, 0.5, 1.0);
static const float4 color_thick = float4(0.0, 0.0, 0.0, 1.0);

float log10(float x) { return log(x) / log(10.0); }

float max2(float2 v) { return max(v.x, v.y); }

[shader("fragment")]
float4 fragmentMain(VertexOutput input) : SV_Target {
    
    float2 dx = float2(ddx(input.v_world_pos.x), ddy(input.v_world_pos.x));
    float2 dy = float2(ddx(input.v_world_pos.z), ddy(input.v_world_pos.z));
    float2 dudv = float2(length(dx), length(dy)) * 4.0;

    float lod = max(0.0, log10(length(dudv) * min_pixel_spacing / cell_size) + 1.0);

    float cell0 = cell_size * pow(10.0, floor(lod));
    float cell1 = cell0 * 10.0;
    float cell2 = cell1 * 10.0;

    float2 mod2 = fmod(input.v_world_pos.xz, cell2) / dudv;
    float lod2a = max2(1.0 - abs(saturate(mod2 * 2.0 - 1.0)));

    float2 mod1 = fmod(input.v_world_pos.xz, cell1) / dudv;
    float lod1a = max2(1.0 - abs(saturate(mod1 * 2.0 - 1.0)));

    float2 mod0 = fmod(input.v_world_pos.xz, cell0) / dudv;
    float lod0a = max2(1.0 - abs(saturate(mod0 * 2.0 - 1.0)));

    float lod_frac = frac(lod);
    float4 color = float4(0.0, 0.0, 0.0, 0.0);

    if (lod2a > 0.0) {
        color = color_thick * lod2a;
    } 
    else if (lod1a > 0.0) {
        color = lerp(color_thin, color_thick, lod_frac) * lod1a;
    } 
    else {
        color = color_thin * (lod0a * (1.0 - lod_frac));
    }

    float dist = length(input.v_world_pos.xz - pc.camera_pos.xz);
    float fade = 1.0 - saturate(dist / pc.grid_size);
    color.a *= fade;

    return color;
}