# Shader Conventions

## Descriptors

For materials descriptor set = 0, binding = 0 will always describe the material data

```glsl
layout(set = 0, binding = 0) uniform MaterialData {
	vec4 base_color;
	float metallic;
	float roughness;
}
u_material_data;

layout(set = 0, binding = 1) uniform sampler2D u_diffuse_texture;
```

```cpp
MaterialDefinition definition;
definition.uniforms = {
    { "base_color", 0, ShaderUniformVariableType::VEC4 },
    { "metallic", 0, ShaderUniformVariableType::FLOAT },
    { "roughness", 0, ShaderUniformVariableType::FLOAT },
    { "u_diffuse_texture", 1, ShaderUniformVariableType::TEXTURE },
};
```