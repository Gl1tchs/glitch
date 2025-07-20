#version 450

layout(location = 0) out vec3 v_world_pos;

layout(push_constant) uniform PushConstants {
	mat4 view_proj;
	vec3 camera_pos;
	float grid_size;
}
pc;

vec3 positions[4] = vec3[](vec3(-1.0, 0.0, -1.0), // bottom left
		vec3(1.0, 0.0, -1.0), // bottom right
		vec3(1.0, 0.0, 1.0), // top right
		vec3(-1.0, 0.0, 1.0) // top left
);

int indices[6] = int[](0, 2, 1, 2, 0, 3);

void main() {
	int idx = indices[gl_VertexIndex];
	vec3 pos = positions[idx] * pc.grid_size;

	pos.x += pc.camera_pos.x;
	pos.z += pc.camera_pos.z;

	v_world_pos = pos;
	gl_Position = pc.view_proj * vec4(pos, 1.0);
}
