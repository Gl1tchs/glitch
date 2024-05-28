#pragma once

#include "gl/renderer/compute.h"

#include "platform/vulkan/vk_pipeline.h"

struct VulkanComputePipelineCreateInfo {
	VkShaderModule shader_module;
	VulkanPipelineLayout layout;
};

struct VulkanComputePipeline {
	VkPipeline pipeline;

	static VulkanComputePipeline create(const VulkanContext& context,
			const VulkanComputePipelineCreateInfo* info);

	static void destroy(const VulkanContext& context,
			const VulkanComputePipeline* pipeline);
};

struct VulkanComputeEffectCreateInfo {
	uint32_t group_count_x;
	uint32_t group_count_y;
	uint32_t group_count_z;
	const char* shader_spv_path;
};

struct VulkanComputeEffect : public ComputeEffect {
	VulkanPipelineLayout pipeline_layout;
	VulkanComputePipeline pipeline;

	uint32_t group_count_x;
	uint32_t group_count_y;
	uint32_t group_count_z;

	virtual ~VulkanComputeEffect() = default;

	static Ref<VulkanComputeEffect> create(const VulkanContext& context,
			const VulkanComputeEffectCreateInfo* info);

	static void destroy(
			const VulkanContext& context, VulkanComputeEffect* effect);
};
