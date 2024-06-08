#pragma once

#include "renderer/types.h"

struct MaterialInstance {
	Pipeline pipeline;
	Shader shader;
	UniformSet uniform_set;
};

struct Material {
	Pipeline pipeline;
	Shader shader;

	struct MaterialResources {
		Image color_image;
		Sampler color_sampler;
	};

	static Ref<Material> create();

	static void destroy(Ref<Material> p_material);

	Ref<MaterialInstance> create_instance(const MaterialResources& resources);

private:
	std::vector<Buffer> allocated_buffers;
};
