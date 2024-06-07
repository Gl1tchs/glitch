#include "platform/vulkan/vk_shader.h"

#include "platform/vulkan/vk_common.h"
#include "platform/vulkan/vk_context.h"

#include <vulkan/vulkan_core.h>

#include <spirv_reflect.h>

namespace vk {

static VkDescriptorType _spv_reflect_descriptor_type_to_vk(
		SpvReflectDescriptorType p_type) {
	switch (p_type) {
		case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:
			return VK_DESCRIPTOR_TYPE_SAMPLER;
		case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
			return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
			return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:
			return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
			return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
		case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
			return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
		case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
			return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
			return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
			return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
			return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
		case SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
			return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		default:
			GL_ASSERT(false, "Unsupported descriptor type.");
			return VK_DESCRIPTOR_TYPE_MAX_ENUM;
	}
}

static void _add_descriptor_set_layout_binding_if_not_exists(uint32_t p_set,
		uint32_t p_binding, VkDescriptorType p_type, ShaderStage p_stage,
		std::unordered_map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>>&
				p_bindings) {
	const auto it = p_bindings.find(p_set);
	if (it != p_bindings.end()) {
		// set exists look for binding

		std::vector<VkDescriptorSetLayoutBinding>& set_bindings = it->second;

		const auto binding_it = std::find_if(set_bindings.begin(),
				set_bindings.end(),
				[=](const VkDescriptorSetLayoutBinding& set_binding) -> bool {
					return set_binding.binding == p_binding;
				});
		if (binding_it != set_bindings.end()) {
			// set, binding already exists now add stage if not exists
			if (!(binding_it->stageFlags & p_stage)) {
				binding_it->stageFlags |= p_stage;
			}
			return;
		}
	}

	VkDescriptorSetLayoutBinding layout_binding = {};
	layout_binding.binding = p_binding;
	layout_binding.descriptorType = p_type;
	layout_binding.descriptorCount = 1;
	layout_binding.stageFlags = p_stage;
	layout_binding.pImmutableSamplers = nullptr;

	p_bindings[p_set].push_back(layout_binding);
}

static void _add_push_constant_range_if_not_exists(uint32_t p_size,
		uint32_t p_offset, ShaderStage stage,
		std::vector<VkPushConstantRange>& p_ranges) {
	const auto it = std::find_if(p_ranges.begin(), p_ranges.end(),
			[=](const VkPushConstantRange& push_constant) -> bool {
				return push_constant.size == p_size &&
						push_constant.offset == p_offset;
			});
	if (it != p_ranges.end()) {
		// push constant already exists now add the stage
		if (!(it->stageFlags & stage)) {
			it->stageFlags |= stage;
		}
		return;
	}

	VkPushConstantRange range = {};
	range.size = p_size;
	range.offset = p_offset;
	range.stageFlags = stage;

	p_ranges.push_back(range);
}

Shader shader_create_from_bytecode(
		Context p_context, const std::vector<SpirvData>& p_shaders) {
	VulkanContext* context = (VulkanContext*)p_context;

	std::vector<VkShaderModule> vk_shaders;

	std::unordered_map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>>
			set_bindings;
	std::vector<VkPushConstantRange> push_constant_ranges;

	for (const auto& shader : p_shaders) {
		SpvReflectShaderModule module = {};
		GL_ASSERT(spvReflectCreateShaderModule(shader.byte_code.size(),
						  shader.byte_code.data(),
						  &module) == SPV_REFLECT_RESULT_SUCCESS);

		uint32_t spv_descriptor_set_count = 0;
		GL_ASSERT(spvReflectEnumerateDescriptorSets(&module,
						  &spv_descriptor_set_count,
						  nullptr) == SPV_REFLECT_RESULT_SUCCESS);

		std::vector<SpvReflectDescriptorSet*> spv_descriptor_sets(
				spv_descriptor_set_count);
		GL_ASSERT(spvReflectEnumerateDescriptorSets(&module,
						  &spv_descriptor_set_count,
						  spv_descriptor_sets.data()) ==
				SPV_REFLECT_RESULT_SUCCESS);

		// add bindings
		for (const auto* descriptor_set : spv_descriptor_sets) {
			for (uint32_t i = 0; i < descriptor_set->binding_count; i++) {
				const SpvReflectDescriptorBinding* binding =
						descriptor_set->bindings[i];

				_add_descriptor_set_layout_binding_if_not_exists(
						descriptor_set->set, binding->binding,
						_spv_reflect_descriptor_type_to_vk(
								binding->descriptor_type),
						shader.stage, set_bindings);
			}
		}

		uint32_t spv_push_constant_count = 0;
		GL_ASSERT(spvReflectEnumeratePushConstantBlocks(&module,
						  &spv_push_constant_count,
						  nullptr) == SPV_REFLECT_RESULT_SUCCESS);

		std::vector<SpvReflectBlockVariable*> spv_push_constants(
				spv_push_constant_count);
		GL_ASSERT(
				spvReflectEnumeratePushConstantBlocks(&module,
						&spv_push_constant_count, spv_push_constants.data()) ==
				SPV_REFLECT_RESULT_SUCCESS);

		for (const auto* push_constant : spv_push_constants) {
			_add_push_constant_range_if_not_exists(push_constant->size,
					push_constant->offset, shader.stage, push_constant_ranges);
		}

		// create a new shader module, using the buffer we loaded
		VkShaderModuleCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		create_info.pNext = nullptr;
		create_info.codeSize = shader.byte_code.size();
		create_info.pCode = shader.byte_code.data();

		VkShaderModule vk_shader = VK_NULL_HANDLE;
		VK_CHECK(vkCreateShaderModule(
				context->device, &create_info, nullptr, &vk_shader));

		vk_shaders.push_back(vk_shader);

		spvReflectDestroyShaderModule(&module);
	}

	std::vector<VkDescriptorSetLayout> descriptor_set_layout;
	for (const auto& [set, bindings] : set_bindings) {
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
			static_cast<uint32_t>(push_constant_ranges.size());
	pipeline_layout_info.pPushConstantRanges = push_constant_ranges.data();

	VkPipelineLayout vk_pipeline_layout;
	VK_CHECK(vkCreatePipelineLayout(context->device, &pipeline_layout_info,
			nullptr, &vk_pipeline_layout));

	std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
	for (size_t i = 0; i < p_shaders.size(); i++) {
		VkPipelineShaderStageCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		create_info.stage =
				static_cast<VkShaderStageFlagBits>(p_shaders[i].stage);
		create_info.pName = "main";
		create_info.module = vk_shaders[i];

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
		vkDestroyShaderModule(context->device,
				shader_info->stage_create_infos[i].module, nullptr);
	}

	VersatileResource::free(context->resources_allocator, shader_info);
}

} //namespace vk
