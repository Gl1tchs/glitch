#include "platform/vulkan/vk_compute.h"

#include "platform/vulkan/vk_init.h"
#include "platform/vulkan/vk_renderer.h"

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

Ref<VulkanComputeEffect> VulkanComputeEffect::create(
		const VulkanContext& context,
		const VulkanComputeEffectCreateInfo* info) {
	Ref<VulkanComputeEffect> effect = create_ref<VulkanComputeEffect>();
	effect->group_count_x = info->group_count_x;
	effect->group_count_y = info->group_count_y;
	effect->group_count_z = info->group_count_z;

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

	VkShaderModule compute_shader;
	GL_ASSERT(vk_load_shader_module_external(
			context.device, info->shader_spv_path, &compute_shader));

	VulkanComputePipelineCreateInfo pipeline_info = {
		.shader_module = compute_shader,
		.layout = effect->pipeline_layout,
	};

	effect->pipeline = VulkanComputePipeline::create(context, &pipeline_info);

	vkDestroyShaderModule(context.device, compute_shader, nullptr);

	return effect;
}

void VulkanComputeEffect::destroy(
		const VulkanContext& context, VulkanComputeEffect* effect) {
	VulkanComputePipeline::destroy(context, &effect->pipeline);

	VulkanPipelineLayout::destroy(context.device, effect->pipeline_layout);
}
