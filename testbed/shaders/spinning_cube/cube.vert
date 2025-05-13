#version 450

layout(location = 0) in vec3 position;

layout(push_constant) uniform constants {
    mat4 view_proj;
    mat4 transform;
};

void main() {
    gl_Position = view_proj * transform * vec4(position, 1.0f);
}
