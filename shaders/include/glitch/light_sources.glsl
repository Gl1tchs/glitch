#ifndef LIGHT_SOURCES_GLSL
#define LIGHT_SOURCES_GLSL

#extension GL_EXT_buffer_reference : require

struct DirectionalLight {
	vec3 direction;
	vec3 color;
};

struct PointLight {
	vec3 position;
	vec3 color;
	float linear;
	float quadratic;
};

struct Spotlight {
	vec3 position;
	vec3 direction;
	vec3 color;
	float cut_off;
};

#endif // LIGHT_SOURCES_GLSL