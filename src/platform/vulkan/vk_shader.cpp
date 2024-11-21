#include "platform/vulkan/vk_backend.h"
#include "glitch/renderer/types.h"

#include <spirv_reflect.h>
#include <vulkan/vulkan_core.h>

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
		uint32_t p_binding, VkDescriptorType p_type,
		uint32_t p_descriptor_count, ShaderStage p_stage,
		std::map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>>&
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
	layout_binding.descriptorCount = p_descriptor_count;
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

template <typename T> void _hash_combine(std::size_t& seed, const T& value) {
	std::hash<T> hasher;
	seed ^= hasher(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

Shader VulkanRenderBackend::shader_create_from_bytecode(
		const std::vector<SpirvData>& p_shaders) {
	std::vector<VkShaderModule> vk_shaders;

	std::map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> set_bindings;
	std::vector<VkPushConstantRange> push_constant_ranges;
	std::vector<ShaderInterfaceVariable> vertex_input_variables;

	for (const auto& shader : p_shaders) {
		SpvReflectShaderModule module = {};

		SpvReflectResult result = spvReflectCreateShaderModule(
				shader.byte_code.size(), shader.byte_code.data(), &module);
		GL_ASSERT(result == SPV_REFLECT_RESULT_SUCCESS);

		// vertex input variables
		if (shader.stage & SHADER_STAGE_VERTEX_BIT) {
			uint32_t input_count = 0;
			GL_ASSERT(spvReflectEnumerateInputVariables(&module, &input_count,
							  nullptr) == SPV_REFLECT_RESULT_SUCCESS);
			std::vector<SpvReflectInterfaceVariable*> inputs(input_count);
			GL_ASSERT(spvReflectEnumerateInputVariables(&module, &input_count,
							  inputs.data()) == SPV_REFLECT_RESULT_SUCCESS);

			// add input variables
			for (const auto* input : inputs) {
				if (input->name == nullptr || input->location == UINT32_MAX) {
					continue;
				}
				
				ShaderInterfaceVariable variable;
				variable.name = input->name;
				variable.location = input->location;
				variable.format = static_cast<DataFormat>(input->format);

				vertex_input_variables.push_back(variable);
			}
		}

		// descriptor sets
		{
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
							binding->count, shader.stage, set_bindings);
				}
			}
		}

		// push constants
		{
			uint32_t spv_push_constant_count = 0;
			GL_ASSERT(spvReflectEnumeratePushConstantBlocks(&module,
							  &spv_push_constant_count,
							  nullptr) == SPV_REFLECT_RESULT_SUCCESS);

			std::vector<SpvReflectBlockVariable*> spv_push_constants(
					spv_push_constant_count);
			GL_ASSERT(spvReflectEnumeratePushConstantBlocks(&module,
							  &spv_push_constant_count,
							  spv_push_constants.data()) ==
					SPV_REFLECT_RESULT_SUCCESS);

			for (const auto* push_constant : spv_push_constants) {
				_add_push_constant_range_if_not_exists(push_constant->size,
						push_constant->offset, shader.stage,
						push_constant_ranges);
			}
		}

		// create a new shader module, using the buffer we loaded
		VkShaderModuleCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		create_info.pNext = nullptr;
		create_info.codeSize = shader.byte_code.size();
		create_info.pCode = shader.byte_code.data();

		VkShaderModule vk_shader = VK_NULL_HANDLE;
		VK_CHECK(vkCreateShaderModule(
				device, &create_info, nullptr, &vk_shader));

		vk_shaders.push_back(vk_shader);

		spvReflectDestroyShaderModule(&module);
	}

	std::vector<VkDescriptorSetLayout> descriptor_set_layouts;
	for (auto& [_, bindings] : set_bindings) {
		std::sort(bindings.begin(), bindings.end(),
				[=](const VkDescriptorSetLayoutBinding& lhs,
						const VkDescriptorSetLayoutBinding& rhs) -> bool {
					return lhs.binding < rhs.binding;
				});

		VkDescriptorSetLayoutCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		create_info.bindingCount = static_cast<uint32_t>(bindings.size());
		create_info.pBindings = bindings.data();

		VkDescriptorSetLayout vk_set;
		VK_CHECK(vkCreateDescriptorSetLayout(
				device, &create_info, nullptr, &vk_set));

		descriptor_set_layouts.push_back(vk_set);
	}

	VkPipelineLayoutCreateInfo pipeline_layout_info = {};
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount =
			static_cast<uint32_t>(descriptor_set_layouts.size());
	pipeline_layout_info.pSetLayouts = descriptor_set_layouts.data();
	pipeline_layout_info.pushConstantRangeCount =
			static_cast<uint32_t>(push_constant_ranges.size());
	pipeline_layout_info.pPushConstantRanges = push_constant_ranges.data();

	VkPipelineLayout vk_pipeline_layout;
	VK_CHECK(vkCreatePipelineLayout(
			device, &pipeline_layout_info, nullptr, &vk_pipeline_layout));

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

	uint32_t push_constant_stages = 0;
	for (const auto& push_constant : push_constant_ranges) {
		push_constant_stages |= push_constant.stageFlags;
	}

	// prepare hash
	size_t shader_hash = 0;
	{
		for (const auto& shader : p_shaders) {
			_hash_combine(shader_hash, shader.stage);
			_hash_combine(shader_hash, shader.byte_code.size());
		}
		for (const auto& [_, bindings] : set_bindings) {
			for (const auto& binding : bindings) {
				_hash_combine(shader_hash, binding.binding);
				_hash_combine(shader_hash, binding.stageFlags);
				_hash_combine(shader_hash, binding.descriptorCount);
				_hash_combine(shader_hash, binding.descriptorType);
			}
		}
		for (const auto& push_constant : push_constant_ranges) {
			_hash_combine(shader_hash, push_constant.stageFlags);
			_hash_combine(shader_hash, push_constant.offset);
			_hash_combine(shader_hash, push_constant.size);
		}
	}

	// Bookkeep
	VulkanShader* shader_info =
			VersatileResource::allocate<VulkanShader>(resources_allocator);
	shader_info->stage_create_infos = shader_stages;
	shader_info->push_constant_stages = push_constant_stages;
	shader_info->descriptor_set_layouts = descriptor_set_layouts;
	shader_info->pipeline_layout = vk_pipeline_layout;
	shader_info->vertex_input_variables = vertex_input_variables;
	shader_info->shader_hash = shader_hash;

	return Shader(shader_info);
}

void VulkanRenderBackend::shader_free(Shader p_shader) {
	VulkanShader* shader_info = (VulkanShader*)p_shader;

	for (size_t i = 0; i < shader_info->descriptor_set_layouts.size(); i++) {
		vkDestroyDescriptorSetLayout(
				device, shader_info->descriptor_set_layouts[i], nullptr);
	}

	vkDestroyPipelineLayout(device, shader_info->pipeline_layout, nullptr);

	for (size_t i = 0; i < shader_info->stage_create_infos.size(); i++) {
		vkDestroyShaderModule(
				device, shader_info->stage_create_infos[i].module, nullptr);
	}

	VersatileResource::free(resources_allocator, shader_info);
}

std::vector<ShaderInterfaceVariable>
VulkanRenderBackend::shader_get_vertex_inputs(Shader p_shader) {
	VulkanShader* shader_info = (VulkanShader*)p_shader;
	return shader_info->vertex_input_variables;
}
