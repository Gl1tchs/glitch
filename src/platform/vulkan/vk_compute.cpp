#include "platform/vulkan/vk_compute.h"

#include "platform/vulkan/vk_init.h"

#include <vulkan/vulkan_core.h>

VulkanComputePipeline VulkanComputePipeline::create(
		const VulkanContext& context,
		const VulkanComputePipelineCreateInfo* info) {
	VkPipelineShaderStageCreateInfo shader_stage_info =
			vkinit::pipeline_shader_stage_create_info(
					VK_SHADER_STAGE_COMPUTE_BIT, info->shader_module);

	VkComputePipelineCreateInfo create_info = {
		.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.stage = shader_stage_info,
		.layout = info->layout.layout,
	};

	VulkanComputePipeline pipeline{ VK_NULL_HANDLE };

	VK_CHECK(vkCreateComputePipelines(context.device, VK_NULL_HANDLE, 1,
			&create_info, nullptr, &pipeline.pipeline));

	return pipeline;
}

void VulkanComputePipeline::destroy(
		const VulkanContext& context, const VulkanComputePipeline* pipeline) {
	vkDestroyPipeline(context.device, pipeline->pipeline, nullptr);
}
