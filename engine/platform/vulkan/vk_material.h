#pragma once

#include "renderer/material.h"

#include "platform/vulkan/vk_buffer.h"
#include "platform/vulkan/vk_context.h"
#include "platform/vulkan/vk_descriptors.h"
#include "platform/vulkan/vk_pipeline.h"

struct VulkanMaterialPipeline {
	VulkanPipeline pipeline;
	VulkanPipelineLayout pipeline_layout;
};

struct VulkanMaterialInstance : public MaterialInstance {
	VulkanMaterialPipeline* pipeline;
	VkDescriptorSet descriptor_set;

	virtual ~VulkanMaterialInstance() = default;
};

struct VulkanMetallicRoughnessMaterial : public MetallicRoughnessMaterial {
	VulkanMaterialPipeline pipeline;
	VkDescriptorSetLayout material_layout;

	~VulkanMetallicRoughnessMaterial() = default;

	static Ref<VulkanMetallicRoughnessMaterial> create(VulkanContext& context);

	static void destroy(
			VulkanContext& context, VulkanMetallicRoughnessMaterial* material);

	Ref<VulkanMaterialInstance> create_instance(
			VulkanContext& context, const MaterialResources& resources);

private:
	DescriptorWriter writer;
	std::vector<VulkanBuffer> allocated_buffers;
};
