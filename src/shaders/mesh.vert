#version 450

#include "input_structures.glsl"

layout(location = 0) out vec3 v_position;
layout(location = 1) out vec3 v_normal;
layout(location = 2) out vec2 v_uv;

void main() {
    Vertex v = push_constants.vertex_buffer.vertices[gl_VertexIndex];

    vec4 frag_pos = push_constants.transform * vec4(v.position, 1.0f);

    // output the position of each vertex
    gl_Position = scene_data.viewproj * frag_pos;

    v_position = frag_pos.xyz;
    v_normal = v.normal;
    v_uv = vec2(v.uv_x, v.uv_y);
}
