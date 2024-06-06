#include "platform/vulkan/vk_shader.h"

#include "platform/vulkan/vk_common.h"
#include "platform/vulkan/vk_context.h"

#include <spirv_cross.hpp>
#include <spirv_glsl.hpp>

namespace vk {

static VkShaderStageFlagBits _get_shader_stage_from_spv_execution_model(
		spv::ExecutionModel execution_model) {
	switch (execution_model) {
		case spv::ExecutionModelVertex:
			return VK_SHADER_STAGE_VERTEX_BIT;
		case spv::ExecutionModelFragment:
			return VK_SHADER_STAGE_FRAGMENT_BIT;
		case spv::ExecutionModelGLCompute:
			return VK_SHADER_STAGE_COMPUTE_BIT;
		case spv::ExecutionModelGeometry:
			return VK_SHADER_STAGE_GEOMETRY_BIT;
		case spv::ExecutionModelTessellationControl:
			return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		case spv::ExecutionModelTessellationEvaluation:
			return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		default:
			GL_ASSERT(false, "Shader stage not supported!");
			return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
	}
}

Shader shader_create_from_bytecode(
		Context p_context, const std::vector<uint32_t>& p_shader_binary) {
	VulkanContext* context = (VulkanContext*)p_context;

	const uint32_t* binptr = p_shader_binary.data();
	const size_t binsize = p_shader_binary.size();

	spirv_cross::Compiler compiler(binptr, binsize);

	const auto resources = compiler.get_shader_resources();

	std::vector<VkDescriptorSetLayoutBinding> bindings;
	const auto add_binding =
			[&](const spirv_cross::SmallVector<spirv_cross::Resource>&
							resources,
					VkDescriptorType type, VkShaderStageFlags stage) -> void {
		for (const auto& resource : resources) {
			uint32_t set = compiler.get_decoration(
					resource.id, spv::DecorationDescriptorSet);
			uint32_t binding = compiler.get_decoration(
					resource.id, spv::DecorationBinding);

			VkDescriptorSetLayoutBinding layout_binding{};
			layout_binding.binding = binding;
			layout_binding.descriptorType = type;
			layout_binding.descriptorCount = 1;
			layout_binding.stageFlags = stage;
			layout_binding.pImmutableSamplers = nullptr;

			bindings.push_back(layout_binding);
		}
	};

	VkShaderStageFlags stage = VK_SHADER_STAGE_ALL;

	// add bindings
	add_binding(resources.sampled_images,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, stage);
	add_binding(
			resources.separate_images, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, stage);
	add_binding(
			resources.storage_images, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, stage);
	add_binding(resources.uniform_buffers, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			stage);
	add_binding(resources.storage_buffers, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			stage);

	std::vector<VkPushConstantRange> push_constants;
	for (const auto& push_constant : resources.push_constant_buffers) {
		const spirv_cross::SPIRType& type =
				compiler.get_type(push_constant.base_type_id);

		size_t size = compiler.get_declared_struct_size(type);
		size_t offset = compiler.type_struct_member_offset(type, 0);

		VkPushConstantRange range = {};
		range.size = size;
		range.offset = offset;
		range.stageFlags = stage;

		push_constants.push_back(range);
	}

	std::vector<VkDescriptorSetLayout> descriptor_set_layout;
	for (const auto& binding : bindings) {
		VkDescriptorSetLayoutCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		create_info.bindingCount = static_cast<uint32_t>(bindings.size());
		create_info.pBindings = bindings.data();

		VkDescriptorSetLayout vk_set;
		VK_CHECK(vkCreateDescriptorSetLayout(
				context->device, &create_info, nullptr, &vk_set));

		descriptor_set_layout.push_back(vk_set);
	}

	VkPipelineLayoutCreateInfo pipeline_layout_info = {};
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount =
			static_cast<uint32_t>(descriptor_set_layout.size());
	pipeline_layout_info.pSetLayouts = descriptor_set_layout.data();
	pipeline_layout_info.pushConstantRangeCount =
			static_cast<uint32_t>(push_constants.size());
	pipeline_layout_info.pPushConstantRanges = push_constants.data();

	VkPipelineLayout vk_pipeline_layout;
	VK_CHECK(vkCreatePipelineLayout(context->device, &pipeline_layout_info,
			nullptr, &vk_pipeline_layout));

	// create a new shader module, using the buffer we loaded
	VkShaderModuleCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_info.pNext = nullptr;
	create_info.codeSize = (uint32_t)p_shader_binary.size();
	create_info.pCode = (uint32_t*)p_shader_binary.data();

	VkShaderModule vk_shader = VK_NULL_HANDLE;
	VK_CHECK(vkCreateShaderModule(
			context->device, &create_info, nullptr, &vk_shader));

	std::vector<VkPipelineShaderStageCreateInfo> shader_stages;

	const auto entry_points = compiler.get_entry_points_and_stages();
	for (const auto& entry_point : entry_points) {
		VkPipelineShaderStageCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		create_info.stage = _get_shader_stage_from_spv_execution_model(
				entry_point.execution_model);
		create_info.pName = entry_point.name.c_str();
		create_info.module = vk_shader;

		shader_stages.push_back(create_info);
	}

	// Bookkeep
	VulkanShader* shader_info = VersatileResource::allocate<VulkanShader>(
			context->resources_allocator);
	shader_info->stage_create_infos = shader_stages;
	shader_info->descriptor_set_layouts = descriptor_set_layout;
	shader_info->pipeline_layout = vk_pipeline_layout;

	return Shader(shader_info);
}

void shader_free(Context p_context, Shader p_shader) {
	VulkanContext* context = (VulkanContext*)p_context;
	VulkanShader* shader_info = (VulkanShader*)p_shader;

	for (size_t i = 0; i < shader_info->descriptor_set_layouts.size(); i++) {
		vkDestroyDescriptorSetLayout(context->device,
				shader_info->descriptor_set_layouts[i], nullptr);
	}

	vkDestroyPipelineLayout(
			context->device, shader_info->pipeline_layout, nullptr);

	for (size_t i = 0; i < shader_info->stage_create_infos.size(); i++) {
		// they are all using the same module so destroy only once
		vkDestroyShaderModule(context->device,
				shader_info->stage_create_infos[i].module, nullptr);
		break;
	}

	VersatileResource::free(context->resources_allocator, shader_info);
}

} //namespace vk
