#pragma once

#include "renderer/image.h"

struct MaterialInstance {
	virtual ~MaterialInstance() = default;
};

struct Material {
	virtual ~Material() = default;
};

struct MetallicRoughnessMaterial : public Material {
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
		Ref<Image> color_image;
		ImageFilteringMode color_filtering;
		Ref<Image> roughness_image;
		ImageFilteringMode roughness_filtering;
	};

	virtual ~MetallicRoughnessMaterial() = default;

	static Ref<MetallicRoughnessMaterial> create();

	static void destroy(Ref<MetallicRoughnessMaterial> material);

	Ref<MaterialInstance> create_instance(const MaterialResources& resources);
};
