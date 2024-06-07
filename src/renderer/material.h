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

	struct MaterialConstants {
		Vec4f color_factors;
		Vec4f metal_rough_factors;
		// padding, we need it anyway because
		// the uniform buffer is reserved
		// for 24 bytes.
		Vec4f padding[4];
	};

	struct MaterialResources {
		MaterialConstants constants;
		uint32_t constants_offset;
		Image color_image;
		ImageFiltering color_filtering;
		Image roughness_image;
		ImageFiltering roughness_filtering;
	};

	static Ref<Material> create(Context p_context);

	static void destroy(Context p_context, Ref<Material> p_material);

	Ref<MaterialInstance> create_instance(
			Context p_context, const MaterialResources& resources);
};
