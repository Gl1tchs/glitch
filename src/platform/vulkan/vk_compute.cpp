#include "platform/vulkan/vk_compute.h"

#include "platform/vulkan/vk_init.h"

VulkanComputePipeline VulkanComputePipeline::create(
		const VulkanContext& context,
		const VulkanComputePipelineCreateInfo* info) {
	VkPipelineShaderStageCreateInfo shader_stage_info =
			vkinit::pipeline_shader_stage_create_info(
					VK_SHADER_STAGE_COMPUTE_BIT, info->shader_module->shader);

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

Ref<VulkanComputeEffectNode> VulkanComputeEffectNode::create(
		const VulkanContext& context,
		const VulkanComputeEffectCreateInfo* info) {
	GL_ASSERT(info->shader, "Invalid compute shader has been provided!");

	Ref<VulkanComputeEffectNode> effect = create_ref<VulkanComputeEffectNode>();
	effect->group_count = info->group_count;

	VkDescriptorSetLayout layouts[] = {
		context.compute_descriptor_layout,
		context.compute_data_descriptor_layout,
	};

	VulkanPipelineLayoutCreateInfo layout_info = {
		.descriptor_set_count = 2,
		.descriptor_sets = layouts,
	};

	effect->pipeline_layout =
			VulkanPipelineLayout::create(context.device, &layout_info);

	VulkanComputePipelineCreateInfo pipeline_info = {
		.shader_module = info->shader,
		.layout = effect->pipeline_layout,
	};

	effect->pipeline = VulkanComputePipeline::create(context, &pipeline_info);

	return effect;
}

void VulkanComputeEffectNode::destroy(
		const VulkanContext& context, const VulkanComputeEffectNode* effect) {
	VulkanComputePipeline::destroy(context, &effect->pipeline);

	VulkanPipelineLayout::destroy(context.device, &effect->pipeline_layout);
}
