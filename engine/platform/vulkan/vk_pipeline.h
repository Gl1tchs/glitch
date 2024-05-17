#pragma once

#include "platform/vulkan/vk_common.h"

enum class VulkanBlendingMode {
	ADDITIVE,
	ALPHA_BLEND,
};

struct VulkanPipelineLayoutCreateInfo {
	uint32_t push_constant_count = 0;
	VkPushConstantRange* push_constants = nullptr;
	uint32_t descriptor_set_count = 0;
	VkDescriptorSetLayout* descriptor_sets = nullptr;
};

struct VulkanPipelineLayout {
	VkPipelineLayout layout;

	VulkanPipelineLayout(
			VkDevice device, const VulkanPipelineLayoutCreateInfo* info);
	~VulkanPipelineLayout();

private:
	VkDevice device;
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

	void enable_blending(VulkanBlendingMode blending_mode);

	void disable_blending();

	void set_color_attachment_format(VkFormat format);

	void set_depth_format(VkFormat format);

	void enable_depthtest(bool depth_write_enable, VkCompareOp op);

	void disable_depthtest();
};

class VulkanPipeline {
public:
	VulkanPipeline(VkDevice device, const VulkanPipelineCreateInfo* info,
			const VulkanPipelineLayout* layout);
	~VulkanPipeline();

	void bind(VkCommandBuffer cmd);

private:
	VkDevice device;

	VkPipeline pipeline;

	friend struct VulkanPipelineCreateInfo;
};

bool vk_load_shader_module(VkDevice device, const char* file_path,
		VkShaderModule* out_shader_module);
