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

	static Ref<Material> create(Context p_context);

	static void destroy(Context p_context, Ref<Material> p_material);

	Ref<MaterialInstance> create_instance(
			Context p_context, const MaterialResources& resources);

private:
	std::vector<Buffer> allocated_buffers;
};
