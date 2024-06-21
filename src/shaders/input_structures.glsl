#ifndef INPUT_STRUCTURES_GLSL
#define INPUT_STRUCTURES_GLSL

#extension GL_EXT_buffer_reference : require

layout(set = 0, binding = 0) uniform SceneData {
    vec4 camera_pos;
    mat4 view;
    mat4 proj;
    mat4 viewproj;
    vec4 sun_direction; // w for power
    vec4 sun_color;
} scene_data;

layout(set = 1, binding = 0) uniform MaterialData {
    vec4 diffuse_factor;
    float shininess;
} material_data;

layout(set = 1, binding = 1) uniform sampler2D diffuse_texture;
layout(set = 1, binding = 2) uniform sampler2D specular_texture;
layout(set = 1, binding = 3) uniform sampler2D normal_texture;

struct Vertex {
    vec3 position;
    float uv_x;
    vec3 normal;
    float uv_y;
};

layout(buffer_reference, std430) readonly buffer VertexBuffer {
    Vertex vertices[];
};

layout(push_constant) uniform constants {
    mat4 transform;
    VertexBuffer vertex_buffer;
} push_constants;

#endif
