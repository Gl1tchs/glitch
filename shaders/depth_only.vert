#pragma 450

#include "vertex_data.glsl"

layout(set = 0, binding = 0) uniform LSP {
    mat4 light_space_matrix;
};

layout(push_constant) uniform constants {
    mat4 transform;
    VertexBuffer vertex_buffer;
} push_constants;

void main() {
    Vertex v = push_constants.vertex_buffer.vertices[gl_VertexIndex];

    gl_Position = light_space_matrix
            * push_constants.transform
            * vec4(v.position, 1.0);
}
