#include "backends/vulkan/vk_material.h"

#include "backends/vulkan/vk_buffer.h"
#include "backends/vulkan/vk_image.h"
#include "backends/vulkan/vk_pipeline.h"
#include "backends/vulkan/vk_shader.h"
#include <vulkan/vulkan_core.h>

Ref<VulkanMetallicRoughnessMaterial> VulkanMetallicRoughnessMaterial::create(
		VulkanContext& context) {
	Ref<VulkanShader> vertex_shader =
			VulkanShader::get(context.device, "mesh.vert.spv");
	GL_ASSERT(vertex_shader, "Failed to load mesh vertex shader!");

	Ref<VulkanShader> fragment_shader =
			VulkanShader::get(context.device, "mesh.frag.spv");
	GL_ASSERT(fragment_shader, "Failed to load mesh fragment shader!");

	VkPushConstantRange matrix_range = {
		.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		.offset = 0,
		.size = sizeof(DrawPushConstants),
	};

	Ref<VulkanMetallicRoughnessMaterial> material =
			create_ref<VulkanMetallicRoughnessMaterial>();

	DescriptorLayoutBuilder layout_builder;
	layout_builder.add_binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	layout_builder.add_binding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	layout_builder.add_binding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

	material->material_layout = layout_builder.build(context.device,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

	VkDescriptorSetLayout descriptors[] = {
		context.scene_data_descriptor_layout,
		material->material_layout,
	};

	VulkanPipelineLayoutCreateInfo mesh_layout_info = {
		.push_constant_count = 1,
		.push_constants = &matrix_range,
		.descriptor_set_count = 2,
		.descriptor_sets = descriptors,
	};

	material->pipeline.pipeline_layout =
			VulkanPipelineLayout::create(context.device, &mesh_layout_info);

	VkFormat color_attachment_formats[] = {
		context.color_attachment_format,
	};

	VulkanPipelineCreateInfo pipeline_info = {
		.vertex_shader = vertex_shader,
		.fragment_shader = fragment_shader,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.polygon_mode = VK_POLYGON_MODE_FILL,
		.cull_mode = VK_CULL_MODE_NONE,
		.front_face = VK_FRONT_FACE_CLOCKWISE,
		.color_attachments = color_attachment_formats,
		.depth_attachment = context.depth_attachment_format,
		.blending_mode = VulkanBlendingMode::NONE,
		.enable_depth_test = true,
		.depth_write_enable = true,
		.depth_op = VK_COMPARE_OP_LESS,
	};

	// finally build the pipeline
	material->pipeline.pipeline = VulkanPipeline::create(context.device,
			&pipeline_info, &material->pipeline.pipeline_layout);

	VulkanShader::destroy(context.device, vertex_shader);
	VulkanShader::destroy(context.device, fragment_shader);

	return material;
}

void VulkanMetallicRoughnessMaterial::destroy(
		VulkanContext& context, VulkanMetallicRoughnessMaterial* material) {
	for (auto buffer : material->allocated_buffers) {
		VulkanBuffer::destroy(context.allocator, buffer);
	}

	vkDestroyDescriptorSetLayout(
			context.device, material->material_layout, nullptr);

	VulkanPipelineLayout::destroy(
			context.device, &material->pipeline.pipeline_layout);
	VulkanPipeline::destroy(context.device, material->pipeline.pipeline);
}

Ref<VulkanMaterialInstance> VulkanMetallicRoughnessMaterial::create_instance(
		VulkanContext& context, const MaterialResources& resources) {
	Ref<VulkanMaterialInstance> instance = create_ref<VulkanMaterialInstance>();
	instance->pipeline = &pipeline;
	instance->descriptor_set = context.descriptor_allocator.allocate(
			context.device, material_layout);

	VulkanBuffer material_constants = VulkanBuffer::create(context.allocator,
			sizeof(MaterialConstants), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VMA_MEMORY_USAGE_CPU_TO_GPU);
	allocated_buffers.push_back(material_constants);

	MaterialConstants* scene_data =
			(MaterialConstants*)material_constants.info.pMappedData;
	*scene_data = resources.constants;

	Ref<VulkanImage> color_image = resources.color_image
			? std::dynamic_pointer_cast<VulkanImage>(resources.color_image)
			: context.error_image;
	VkSampler color_sampler = context.get_sampler(resources.color_filtering);

	Ref<VulkanImage> roughness_image = resources.roughness_image
			? std::dynamic_pointer_cast<VulkanImage>(resources.roughness_image)
			: context.white_image;
	VkSampler roughness_sampler =
			context.get_sampler(resources.roughness_filtering);

	writer.clear();
	writer.write_buffer(0, material_constants.buffer, sizeof(MaterialConstants),
			resources.constants_offset, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	writer.write_image(1, color_image->image_view, color_sampler,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	writer.write_image(2, roughness_image->image_view, roughness_sampler,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

	writer.update_set(context.device, instance->descriptor_set);

	return instance;
}