#ifndef COMMON_GLSL
#define COMMON_GLSL

#include "glitch/light_sources.glsl"

#extension GL_EXT_buffer_reference : require

struct MeshVertex {
    vec3 position;
    float uv_x;
    vec3 normal;
    float uv_y;
};

layout(buffer_reference, std430) readonly buffer VertexBuffer {
    MeshVertex vertices[];
};

layout(buffer_reference, std430) readonly buffer SceneBuffer {
    mat4 view_projection;

    vec4 camera_position;

    int num_point_lights;

    DirectionalLight directional_light;
    PointLight point_lights[16];
};

layout(push_constant, std430) uniform constants {
    mat4 transform;
    VertexBuffer vertex_buffer;
    SceneBuffer scene_buffer;
}
u_push_constants;

layout(set = 0, binding = 0) uniform MaterialData {
    vec4 base_color;
    float metallic;
    float roughness;
}
u_material_data;

layout(set = 0, binding = 1) uniform sampler2D u_diffuse_texture;
layout(set = 0, binding = 2) uniform sampler2D u_normal_texture;
layout(set = 0, binding = 3) uniform sampler2D u_metallic_roughness_texture;
layout(set = 0, binding = 4) uniform sampler2D u_ambient_occlusion_texture;

#endif // COMMON_GLSL
