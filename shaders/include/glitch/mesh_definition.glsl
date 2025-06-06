#ifndef MESH_DEFINITION_GLSL
#define MESH_DEFINITION_GLSL

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
	vec3 camera_position;
};

layout(push_constant, std430) uniform constants {
	VertexBuffer vertex_buffer;
	SceneBuffer scene_buffer;
	mat4 transform;
}
u_push_constants;

#endif // MESH_DEFINITION_GLSL
