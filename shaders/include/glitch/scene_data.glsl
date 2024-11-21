#ifndef SCENE_DATA_GLSL
#define SCENE_DATA_GLSL

layout (set = 0, binding = 0) uniform SceneData {
    mat4 view_projection;
    vec3 camera_position;
} u_scene_data;

#endif // SCENE_DATA_GLSL
