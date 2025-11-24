module light_sources;

public struct DirectionalLight
{
    public float4 direction;
    public float4 color;
};

public struct PointLight
{
    public float4 position;
    public float4 color;
    public float linear;
    public float quadratic;
    // Padding might be needed here depending on your C++ alignment, 
    // but Slang handles std430 layout rules automatically in most cases.
    float _pad0; 
    float _pad1; 
};