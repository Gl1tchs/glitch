#pragma once

#include "core/color.h"

#include "renderer/types.h"

struct MaterialInstance {
	Pipeline pipeline;
	Shader shader;
	UniformSet uniform_set;
};

struct MaterialConstants {
	Color diffuse_factor = COLOR_WHITE;
	float shininess = 1.0f;

private:
	float __padding[18];
};

struct MaterialResources {
	MaterialConstants constants;
	Image diffuse_image = GL_NULL_HANDLE;
	Image specular_image = GL_NULL_HANDLE;
	Image normal_image = GL_NULL_HANDLE;
	Sampler sampler = GL_NULL_HANDLE;
};

struct Material {
	Pipeline pipeline;
	Shader shader;

	static Ref<Material> create();

	static void destroy(Ref<Material> p_material);

	Ref<MaterialInstance> create_instance(const MaterialResources& resources);

private:
	std::vector<Buffer> allocated_buffers;
};
