#version 450

layout(set = 0, binding = 0) uniform CameraData {
    mat4 view;
    mat4 proj;
    float near_plane;
    float far_plane;
} camera_data;

layout(location = 0) out vec3 v_near_point;
layout(location = 1) out vec3 v_far_point;

const vec3 GRID_PLANE[6] = vec3[](
        vec3(1, 1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
        vec3(-1, -1, 0), vec3(1, 1, 0), vec3(1, -1, 0)
    );

vec3 unproject_point(float x, float y, float z, mat4 view, mat4 proj) {
    mat4 view_inv = inverse(view);
    mat4 proj_inv = inverse(proj);

    vec4 unprojected_point = view_inv * proj_inv * vec4(x, y, z, 1.0);

    return unprojected_point.xyz / unprojected_point.w;
}

void main() {
    vec3 p = GRID_PLANE[gl_VertexIndex].xyz;

    v_near_point = unproject_point(p.x, p.y, 0.0, camera_data.view,
            camera_data.proj).xyz;
    v_far_point = unproject_point(p.x, p.y, 1.0, camera_data.view,
            camera_data.proj).xyz;

    gl_Position = vec4(p, 1.0);
}
