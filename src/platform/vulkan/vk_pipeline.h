#pragma once

#include "platform/vulkan/vk_common.h"

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
	std::vector<VkPipelineShaderStageCreateInfo> shader_stages;

	VkPipelineInputAssemblyStateCreateInfo input_assembly;
	VkPipelineRasterizationStateCreateInfo rasterizer;
	VkPipelineColorBlendAttachmentState color_blend_attachment;
	VkPipelineMultisampleStateCreateInfo multisampling;
	VkPipelineDepthStencilStateCreateInfo depth_stencil;
	VkPipelineRenderingCreateInfo render_info;
	VkFormat color_attachment_format;

	VulkanPipelineCreateInfo();

	void reset();

	void set_shaders(
			VkShaderModule vertex_shader, VkShaderModule fragment_shader);

	void set_input_topology(VkPrimitiveTopology topology);

	void set_polygon_mode(VkPolygonMode mode);

	void set_cull_mode(VkCullModeFlags cull_mode, VkFrontFace front_face);

	void set_multisampling_none();

	void set_blending(VulkanBlendingMode blending_mode);

	void set_color_attachment_format(VkFormat format);

	void set_depth_format(VkFormat format);

	void enable_depthtest(bool depth_write_enable, VkCompareOp op);

	void disable_depthtest();
};

struct VulkanPipeline {
	VkPipeline pipeline;

	static VulkanPipeline create(VkDevice device,
			const VulkanPipelineCreateInfo* info,
			const VulkanPipelineLayout* layout);
	static void destroy(VkDevice device, VulkanPipeline& pipeline);
};

/**
 * @brief loads shader module from bundled header
 */
bool vk_load_shader_module(VkDevice device, const char* file_path,
		VkShaderModule* out_shader_module);

/**
 * @brief loads shader module from external file
 */
bool vk_load_shader_module_external(VkDevice device, const char* file_path,
		VkShaderModule* out_shader_module);
