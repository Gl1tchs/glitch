#ifndef LIGHT_SOURCES_GLSL
#define LIGHT_SOURCES_GLSL

#extension GL_EXT_buffer_reference : require

struct DirectionalLight {
	vec4 direction;
	vec4 color;
};

struct PointLight {
	vec4 position;
	vec4 color;
	float linear;
	float quadratic;
};

#endif // LIGHT_SOURCES_GLSL