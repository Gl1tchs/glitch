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
	const char* shader_spv_path;
	Vec3u group_count;
};

struct VulkanComputeEffectNode : public ComputeEffectNode {
	VulkanPipelineLayout pipeline_layout;
	VulkanComputePipeline pipeline;

	Vec3u group_count;

	virtual ~VulkanComputeEffectNode() = default;

	static Ref<VulkanComputeEffectNode> create(const VulkanContext& context,
			const VulkanComputeEffectCreateInfo* info);

	static void destroy(const VulkanContext& context,
			const VulkanComputeEffectNode* effect);
};
