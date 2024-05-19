#include "platform/vulkan/vk_material.h"

#include "platform/vulkan/vk_pipeline.h"
#include <vulkan/vulkan_core.h>

VulkanMetallicRoughnessMaterial VulkanMetallicRoughnessMaterial::create(
		const VulkanContext& context) {
	VkShaderModule vertex_shader;
	GL_ASSERT(vk_load_shader_module(
					  context.device, "mesh.vert.spv", &vertex_shader),
			"Failed to load mesh vertex shader!");

	VkShaderModule fragment_shader;
	GL_ASSERT(vk_load_shader_module(
					  context.device, "mesh.frag.spv", &fragment_shader),
			"Failed to load mesh fragment shader!");

	VkPushConstantRange matrix_range = {
		.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		.offset = 0,
		.size = sizeof(DrawPushConstants),
	};

	VulkanMetallicRoughnessMaterial material{};

	DescriptorLayoutBuilder layout_builder;
	layout_builder.add_binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	layout_builder.add_binding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	layout_builder.add_binding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

	material.material_layout = layout_builder.build(context.device,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

	VulkanPipelineLayoutCreateInfo mesh_layout_info = {
		.push_constant_count = 1,
		.push_constants = &matrix_range,
		.descriptor_set_count = 1,
		.descriptor_sets = &material.material_layout,
	};

	material.pipeline.pipeline_layout =
			VulkanPipelineLayout::create(context.device, &mesh_layout_info);

	VulkanPipelineCreateInfo pipeline_info;
	pipeline_info.set_shaders(vertex_shader, fragment_shader);
	pipeline_info.set_input_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	pipeline_info.set_polygon_mode(VK_POLYGON_MODE_FILL);
	pipeline_info.set_cull_mode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
	pipeline_info.set_multisampling_none();
	pipeline_info.enable_blending(VulkanBlendingMode::ADDITIVE);
	pipeline_info.enable_depthtest(true, VK_COMPARE_OP_GREATER_OR_EQUAL);

	// render format
	pipeline_info.set_color_attachment_format(context.color_attachment_format);
	pipeline_info.set_depth_format(context.depth_attachment_format);

	// finally build the pipeline
	material.pipeline.pipeline = VulkanPipeline::create(
			context.device, &pipeline_info, &material.pipeline.pipeline_layout);

	vkDestroyShaderModule(context.device, fragment_shader, nullptr);
	vkDestroyShaderModule(context.device, vertex_shader, nullptr);

	return material;
}

void VulkanMetallicRoughnessMaterial::destroy(
		VkDevice device, VulkanMetallicRoughnessMaterial& material) {
	vkDestroyDescriptorSetLayout(device, material.material_layout, nullptr);

	VulkanPipelineLayout::destroy(device, material.pipeline.pipeline_layout);
	VulkanPipeline::destroy(device, material.pipeline.pipeline);
}

VulkanMaterialInstance VulkanMetallicRoughnessMaterial::write_material(
		VkDevice device, const MaterialResources& resources,
		VulkanDescriptorAllocator& descriptor_allocator) {
	VulkanMaterialInstance instance = {
		.pipeline = &pipeline,
		.descriptor_set =
				descriptor_allocator.allocate(device, material_layout),
	};

	writer.clear();
	writer.write_buffer(0, resources.data_buffer, sizeof(MaterialConstants),
			resources.data_buffer_offset, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	writer.write_image(1, resources.color_image.image_view,
			resources.color_sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	writer.write_image(2, resources.roughness_image.image_view,
			resources.roughness_sampler,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

	writer.update_set(device, instance.descriptor_set);

	return instance;
}
