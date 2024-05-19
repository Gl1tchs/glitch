#version 450
#extension GL_EXT_buffer_reference : require

#include "input_structures.glsl"

layout(location = 0) out vec3 out_color;
layout(location = 1) out vec2 out_uv;

struct Vertex {
    vec3 position;
    float uv_x;
    vec3 normal;
    float uv_y;
    vec4 color;
};

layout(buffer_reference, std430) readonly buffer VertexBuffer {
    Vertex vertices[];
};

// push constants
layout(push_constant) uniform constants {
    VertexBuffer vertex_buffer;
} push_constants;

void main() {
    Vertex v = push_constants.vertex_buffer.vertices[gl_VertexIndex];

    // output the position of each vertex
    gl_Position = vec4(v.position, 1.0f);

    out_color = v.color.xyz * material_data.color_factors.xyz;
    out_uv = vec2(v.uv_x, v.uv_y);
}
