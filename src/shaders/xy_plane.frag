#version 450

layout(set = 0, binding = 0) uniform CameraData {
    mat4 view;
    mat4 proj;
    float near_plane;
    float far_plane;
} camera_data;

layout(location = 0) in vec3 v_near_point;
layout(location = 1) in vec3 v_far_point;

layout(location = 0) out vec4 o_color;

vec4 grid(vec3 frag_pos, float scale) {
    vec2 coord = frag_pos.xz * scale; // use the scale variable to set the distance between the lines
    vec2 derivative = fwidth(coord);
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;

    float line = min(grid.x, grid.y);
    float minimumz = min(derivative.y, 1);
    float minimumx = min(derivative.x, 1);

    vec4 color = vec4(0.2, 0.2, 0.2, 1.0 - min(line, 1.0));

    // z axis
    if (frag_pos.x > -0.1 * minimumx && frag_pos.x < 0.1 * minimumx)
        color.z = 1.0;
    // x axis
    if (frag_pos.z > -0.1 * minimumz && frag_pos.z < 0.1 * minimumz)
        color.x = 1.0;

    return color;
}

float compute_depth(vec3 pos) {
    vec4 clip_space_pos = camera_data.proj * camera_data.view * vec4(pos.xyz, 1.0);
    return (clip_space_pos.z / clip_space_pos.w);
}

float compute_linear_depth(vec3 pos) {
    float near = camera_data.near_plane;
    float far = camera_data.far_plane;

    vec4 clip_space_pos = camera_data.proj * camera_data.view * vec4(pos.xyz, 1.0);

    float clip_space_depth = (clip_space_pos.z / clip_space_pos.w) * 2.0 - 1.0;

    float linear_depth = (2.0 * near * far) / (far + near - clip_space_depth * (far - near));

    return linear_depth / far;
}

void main() {
    float t = -v_near_point.y / (v_far_point.y - v_near_point.y);

    vec3 frag_pos = v_near_point + t * (v_far_point - v_near_point);

    gl_FragDepth = compute_depth(frag_pos);

    float linear_depth = compute_linear_depth(frag_pos);
    float fading = max(0, (0.5 - linear_depth));

    o_color = (grid(frag_pos, 10)
            + grid(frag_pos, 1)) * float(t > 0);
    o_color.a *= fading;
}
