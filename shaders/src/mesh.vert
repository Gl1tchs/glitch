#version 450

#include "glitch/uniforms.glsl"

layout(location = 0) out vec3 v_position;
layout(location = 1) out vec3 v_normal;
layout(location = 2) out vec2 v_uv;

void main() {
	MeshVertex v = u_push_constants.vertex_buffer.vertices[gl_VertexIndex];
	SceneBuffer scene_data = u_push_constants.scene_buffer;

	vec4 frag_pos = vec4(v.position, 1.0f);

	gl_Position = scene_data.view_projection * frag_pos;

	v_position = frag_pos.xyz;
	v_normal = v.normal;
	v_uv = vec2(v.uv_x, v.uv_y);
}
