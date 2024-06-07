#version 450

#include "input_structures.glsl"

const vec4 VERTICES[3] = vec4[3](
        vec4(-0.5, -0.5, 0.0, 1.0),
        vec4(0.5, -0.5, 0.0, 1.0),
        vec4(0.0, 0.5, 0.0, 1.0)
    );

const vec4 COLORS[3] = vec4[3](
        vec4(1.0, 0.0, 0.0, 1.0),
        vec4(0.0, 1.0, 0.0, 1.0),
        vec4(0.0, 0.0, 1.0, 1.0)
    );

layout(location = 0) out vec4 v_frag_color;

void main() {
    gl_Position = scene_data.viewproj * VERTICES[gl_VertexIndex];

    v_frag_color = COLORS[gl_VertexIndex];
}
