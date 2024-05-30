#pragma once

#include "platform/vulkan/vk_common.h"

#include "platform/vulkan/vk_shader.h"

enum class VulkanBlendingMode {
	ADDITIVE,
	ALPHA_BLEND,
	NONE,
};

struct VulkanPipelineLayoutCreateInfo {
	uint32_t push_constant_count = 0;
	VkPushConstantRange* push_constants = nullptr;
	uint32_t descriptor_set_count = 0;
	VkDescriptorSetLayout* descriptor_sets = nullptr;
};

/**
 * @brief thin wrapper around VkPipelineLayout
 */
struct VulkanPipelineLayout {
	VkPipelineLayout layout;

	static VulkanPipelineLayout create(
			VkDevice device, const VulkanPipelineLayoutCreateInfo* info);

	static void destroy(VkDevice device, const VulkanPipelineLayout* layout);
};

struct VulkanPipelineCreateInfo {
	Ref<VulkanShader> vertex_shader;
	Ref<VulkanShader> fragment_shader;

	VkPrimitiveTopology topology;
	VkPolygonMode polygon_mode;

	// culling
	VkCullModeFlags cull_mode;
	VkFrontFace front_face;

	// no multisampling

	std::span<VkFormat> color_attachments;
	VkFormat depth_attachment;

	VulkanBlendingMode blending_mode;

	bool enable_depth_test;
	bool depth_write_enable;
	VkCompareOp depth_op;
};

struct VulkanPipeline {
	VkPipeline pipeline;

	static VulkanPipeline create(VkDevice device,
			const VulkanPipelineCreateInfo* info,
			const VulkanPipelineLayout* layout);
	static void destroy(VkDevice device, VulkanPipeline& pipeline);
};
