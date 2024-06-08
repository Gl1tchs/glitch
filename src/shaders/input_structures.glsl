#ifndef INPUT_STRUCTURES_GLSL
#define INPUT_STRUCTURES_GLSL

layout(set = 0, binding = 0) uniform SceneData {
    vec4 camera_pos;
    mat4 view;
    mat4 proj;
    mat4 viewproj;
    vec4 sun_direction; // w for power
    vec4 sun_color;
} scene_data;

layout(set = 1, binding = 0) uniform sampler2D color_tex;

#endif
