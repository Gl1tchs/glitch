#ifndef MESH_DATA_GLSL
#define MESH_DATA_GLSL

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

layout(push_constant) uniform constants {
    VertexBuffer vertex_buffer;
} u_push_constants;

#endif // MESH_DATA_GLSL
