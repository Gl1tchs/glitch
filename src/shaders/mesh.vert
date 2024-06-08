#version 450
#extension GL_EXT_buffer_reference : require

#include "input_structures.glsl"

layout(location = 0) out vec3 o_position;
layout(location = 1) out vec3 o_normal;
layout(location = 2) out vec2 o_uv;

struct Vertex {
    vec3 position;
    float uv_x;
    vec3 normal;
    float uv_y;
};

layout(buffer_reference, std430) readonly buffer VertexBuffer {
    Vertex vertices[];
};

// push constants
layout(push_constant) uniform constants {
    mat4 transform;
    VertexBuffer vertex_buffer;
} push_constants;

void main() {
    Vertex v = push_constants.vertex_buffer.vertices[gl_VertexIndex];

    vec4 frag_pos = push_constants.transform * vec4(v.position, 1.0f);

    // output the position of each vertex
    gl_Position = scene_data.viewproj * frag_pos;

    o_position = frag_pos.xyz;
    o_normal = v.normal;
    o_uv = vec2(v.uv_x, v.uv_y);
}
