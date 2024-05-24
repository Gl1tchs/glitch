#pragma once

#include "platform/vulkan/vk_common.h"

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
