#pragma once

#include "platform/vulkan/vk_buffer.h"
#include "platform/vulkan/vk_common.h"

#include "platform/vulkan/vk_context.h"
#include "platform/vulkan/vk_descriptors.h"
#include "platform/vulkan/vk_image.h"
#include "platform/vulkan/vk_pipeline.h"

struct VulkanMaterialPipeline {
	VulkanPipeline pipeline;
	VulkanPipelineLayout pipeline_layout;
};

struct VulkanMaterialInstance {
	VulkanMaterialPipeline* pipeline;
	VkDescriptorSet descriptor_set;
};

struct VulkanMetallicRoughnessMaterial {
	VulkanMaterialPipeline pipeline;
	VkDescriptorSetLayout material_layout;

	struct MaterialConstants {
		Vector4f color_factors;
		Vector4f metal_rough_factors;
		// padding, we need it anyway because
		// the uniform buffer is reserved
		// for 24 bytes.
		Vector4f padding[4];
	};

	struct MaterialResources {
		VulkanImage color_image;
		VkSampler color_sampler;
		VulkanImage roughness_image;
		VkSampler roughness_sampler;
		VkBuffer data_buffer;
		uint32_t data_buffer_offset;
	};

	static VulkanMetallicRoughnessMaterial create(const VulkanContext& context);

	static void destroy(
			VkDevice device, VulkanMetallicRoughnessMaterial& material);

	VulkanMaterialInstance write_material(VkDevice device,
			const MaterialResources& resources,
			VulkanDescriptorAllocator& descriptor_allocator);

private:
	DescriptorWriter writer;
};
