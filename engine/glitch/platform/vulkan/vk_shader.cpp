#include "glitch/platform/vulkan/vk_backend.h"
#include "glitch/renderer/types.h"

#include <spirv_reflect.h>
#include <vulkan/vulkan_core.h>

namespace gl {

static DataFormat _map_reflect_format_to_vulkan(SpvReflectFormat fmt) {
	switch (fmt) {
		case SPV_REFLECT_FORMAT_R32_UINT:
			return DataFormat::R32_UINT;
		case SPV_REFLECT_FORMAT_R32_SINT:
			return DataFormat::R32_SINT;
		case SPV_REFLECT_FORMAT_R32_SFLOAT:
			return DataFormat::R32_SFLOAT;
		case SPV_REFLECT_FORMAT_R32G32_UINT:
			return DataFormat::R32G32_UINT;
		case SPV_REFLECT_FORMAT_R32G32_SINT:
			return DataFormat::R32G32_SINT;
		case SPV_REFLECT_FORMAT_R32G32_SFLOAT:
			return DataFormat::R32G32_SFLOAT;
		case SPV_REFLECT_FORMAT_R32G32B32_UINT:
			return DataFormat::R32G32B32_UINT;
		case SPV_REFLECT_FORMAT_R32G32B32_SINT:
			return DataFormat::R32G32B32_SINT;
		case SPV_REFLECT_FORMAT_R32G32B32_SFLOAT:
			return DataFormat::R32G32B32_SFLOAT;
		case SPV_REFLECT_FORMAT_R32G32B32A32_UINT:
			return DataFormat::R32G32B32A32_UINT;
		case SPV_REFLECT_FORMAT_R32G32B32A32_SINT:
			return DataFormat::R32G32B32A32_SINT;
		case SPV_REFLECT_FORMAT_R32G32B32A32_SFLOAT:
			return DataFormat::R32G32B32A32_SFLOAT;
		default:
			return DataFormat::UNDEFINED;
	}
}

template <typename T> void _hash_combine(std::size_t& seed, const T& value) {
	std::hash<T> hasher;
	seed ^= hasher(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

Shader VulkanRenderBackend::shader_create_from_bytecode(
		const std::vector<uint32_t>& byte_code, ShaderStageFlags shader_stages) {
	if (byte_code.empty()) {
		GL_LOG_ERROR("[VULKAN] Shader creation failed: Bytecode is empty.");
		return nullptr;
	}

	SpvReflectShaderModule module;
	SpvReflectResult result = spvReflectCreateShaderModule(
			byte_code.size() * sizeof(uint32_t), byte_code.data(), &module);
	if (result != SPV_REFLECT_RESULT_SUCCESS) {
		GL_LOG_ERROR("[VULKAN] Failed to reflect SPIR-V shader.");
		return nullptr;
	}

	VkShaderModule vk_module = VK_NULL_HANDLE;
	std::vector<VkPipelineShaderStageCreateInfo> temp_stage_infos;
	std::vector<VkDescriptorSetLayout> temp_layouts;
	VkPipelineLayout temp_pipeline_layout = VK_NULL_HANDLE;
	std::vector<ShaderDescriptorReflection> temp_reflection_descriptors;
	std::vector<ShaderPushConstantReflection> temp_reflection_pc;
	std::vector<ShaderInterfaceVariable> temp_vertex_inputs;

	bool has_vertex = (shader_stages & SHADER_STAGE_VERTEX);
	bool has_fragment = (shader_stages & SHADER_STAGE_FRAGMENT);
	bool has_compute = (shader_stages & SHADER_STAGE_FRAGMENT);

	if (has_vertex && spvReflectGetEntryPoint(&module, "vertexMain") == nullptr) {
		GL_LOG_ERROR("[VULKAN] Vertex stage requested but 'vertexMain' not found.");
		spvReflectDestroyShaderModule(&module);
		return nullptr;
	}
	if (has_fragment && spvReflectGetEntryPoint(&module, "fragmentMain") == nullptr) {
		GL_LOG_ERROR("[VULKAN] Fragment stage requested but 'fragmentMain' not found.");
		spvReflectDestroyShaderModule(&module);
		return nullptr;
	}
	if (has_compute && spvReflectGetEntryPoint(&module, "computeMain") == nullptr) {
		GL_LOG_ERROR("[VULKAN] Compute stage requested but 'computeMain' not found.");
		spvReflectDestroyShaderModule(&module);
		return nullptr;
	}

	VkShaderModuleCreateInfo create_info = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
	create_info.codeSize = byte_code.size() * sizeof(uint32_t);
	create_info.pCode = byte_code.data();

	if (vkCreateShaderModule(device, &create_info, nullptr, &vk_module) != VK_SUCCESS) {
		GL_LOG_ERROR("[VULKAN] vkCreateShaderModule failed.");
		spvReflectDestroyShaderModule(&module);
		return nullptr;
	}

	if (has_vertex) {
		VkPipelineShaderStageCreateInfo vert_info = {
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO
		};
		vert_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vert_info.module = vk_module;
		vert_info.pName = "vertexMain";
		temp_stage_infos.push_back(vert_info);
	}
	if (has_fragment) {
		VkPipelineShaderStageCreateInfo frag_info = {
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO
		};
		frag_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		frag_info.module = vk_module;
		frag_info.pName = "fragmentMain";
		temp_stage_infos.push_back(frag_info);
	}
	if (has_compute) {
		VkPipelineShaderStageCreateInfo comp_info = {
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO
		};
		comp_info.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		comp_info.module = vk_module;
		comp_info.pName = "computeMain";
		temp_stage_infos.push_back(comp_info);
	}

	// 5. Reflection: Merge Resources
	// We need to reflect based on specific entry points to avoid pulling in unused resources
	// Map: Set -> Binding -> VkBinding
	std::map<uint32_t, std::map<uint32_t, VkDescriptorSetLayoutBinding>> merged_bindings;

	auto process_entry_point = [&](const char* entry_point, VkShaderStageFlagBits vk_stage) {
		// Descriptors
		uint32_t count = 0;
		spvReflectEnumerateEntryPointDescriptorSets(&module, entry_point, &count, nullptr);
		std::vector<SpvReflectDescriptorSet*> sets(count);
		spvReflectEnumerateEntryPointDescriptorSets(&module, entry_point, &count, sets.data());

		for (auto* set : sets) {
			for (uint32_t i = 0; i < set->binding_count; i++) {
				SpvReflectDescriptorBinding* b = set->bindings[i];
				VkDescriptorSetLayoutBinding& binding_entry = merged_bindings[set->set][b->binding];

				binding_entry.binding = b->binding;
				binding_entry.descriptorType = static_cast<VkDescriptorType>(b->descriptor_type);
				binding_entry.descriptorCount = 1;
				for (uint32_t d = 0; d < b->array.dims_count; ++d)
					binding_entry.descriptorCount *= b->array.dims[d];
				binding_entry.stageFlags |= vk_stage; // Merge flags
			}
		}

		// Inputs (Vertex only)
		if (vk_stage == VK_SHADER_STAGE_VERTEX_BIT) {
			uint32_t in_count = 0;
			spvReflectEnumerateEntryPointInputVariables(&module, entry_point, &in_count, nullptr);
			std::vector<SpvReflectInterfaceVariable*> inputs(in_count);
			spvReflectEnumerateEntryPointInputVariables(
					&module, entry_point, &in_count, inputs.data());

			for (auto* var : inputs) {
				if (var->decoration_flags & SPV_REFLECT_DECORATION_BUILT_IN)
					continue;
				ShaderInterfaceVariable v_in;
				v_in.location = var->location;
				v_in.format = _map_reflect_format_to_vulkan(var->format);
				v_in.name = var->name;
				temp_vertex_inputs.push_back(v_in);
			}
		}
	};

	if (has_vertex) {
		process_entry_point("vertexMain", VK_SHADER_STAGE_VERTEX_BIT);
	}
	if (has_fragment) {
		process_entry_point("fragmentMain", VK_SHADER_STAGE_FRAGMENT_BIT);
	}
	if (has_compute) {
		process_entry_point("computeMain", VK_SHADER_STAGE_COMPUTE_BIT);
	}

	for (auto& [set_id, bindings] : merged_bindings) {
		std::vector<VkDescriptorSetLayoutBinding> vk_bindings;
		ShaderDescriptorReflection reflect_desc;
		reflect_desc.set_number = set_id;

		for (auto& [binding_id, binding_info] : bindings) {
			vk_bindings.push_back(binding_info);

			ShaderReflectionBinding rb;
			rb.binding_point = binding_id;
			rb.descriptor_count = binding_info.descriptorCount;
			rb.type = static_cast<ShaderUniformType>(binding_info.descriptorType);
			reflect_desc.bindings.push_back(rb);
			// TODO! size
		}

		VkDescriptorSetLayoutCreateInfo layout_info = {
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO
		};
		layout_info.bindingCount = (uint32_t)vk_bindings.size();
		layout_info.pBindings = vk_bindings.data();

		VkDescriptorSetLayout layout;
		if (vkCreateDescriptorSetLayout(device, &layout_info, nullptr, &layout) != VK_SUCCESS) {
			// Cleanup
			vkDestroyShaderModule(device, vk_module, nullptr);
			for (auto l : temp_layouts)
				vkDestroyDescriptorSetLayout(device, l, nullptr);
			spvReflectDestroyShaderModule(&module);
			return nullptr;
		}
		temp_layouts.push_back(layout);
		temp_reflection_descriptors.push_back(reflect_desc);
	}

	// Push Constants (Simplified Merge)
	std::vector<VkPushConstantRange> vk_ranges;
	// Reset to get all push constants across module
	// NOTE: A better approach scans per entry point and merges ranges,
	// but often PC ranges are defined globally in GLSL.
	uint32_t pc_count = 0;
	spvReflectEnumeratePushConstantBlocks(&module, &pc_count, nullptr);
	std::vector<SpvReflectBlockVariable*> pcs(pc_count);
	spvReflectEnumeratePushConstantBlocks(&module, &pc_count, pcs.data());

	VkShaderStageFlags push_constant_stages;
	for (auto* pc : pcs) {
		VkPushConstantRange range = {};
		range.offset = pc->offset;
		range.size = pc->size;
		range.stageFlags = 0;

		// Check usage in stages
		// (This relies on spvReflect tracking usage, which might require per-entry-point checks
		// again if the module is complex, but we accept all valid PC blocks here for simplicity).
		if (has_vertex) {
			push_constant_stages |= VK_SHADER_STAGE_VERTEX_BIT;
			range.stageFlags |= VK_SHADER_STAGE_VERTEX_BIT;
		}
		if (has_fragment) {
			push_constant_stages |= VK_SHADER_STAGE_FRAGMENT_BIT;
			range.stageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;
		}
		if (has_compute) {
			push_constant_stages |= VK_SHADER_STAGE_COMPUTE_BIT;
			range.stageFlags |= VK_SHADER_STAGE_COMPUTE_BIT;
		}

		vk_ranges.push_back(range);

		ShaderPushConstantReflection pc_ref;
		pc_ref.name = pc->name;
		pc_ref.offset = pc->offset;
		pc_ref.size = pc->size;
		pc_ref.stage_flags = shader_stages;
		temp_reflection_pc.push_back(pc_ref);
	}

	// Create Pipeline Layout
	VkPipelineLayoutCreateInfo pipeline_info = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	pipeline_info.setLayoutCount = (uint32_t)temp_layouts.size();
	pipeline_info.pSetLayouts = temp_layouts.data();
	pipeline_info.pushConstantRangeCount = (uint32_t)vk_ranges.size();
	pipeline_info.pPushConstantRanges = vk_ranges.data();

	if (vkCreatePipelineLayout(device, &pipeline_info, nullptr, &temp_pipeline_layout) !=
			VK_SUCCESS) {
		// Cleanup
		vkDestroyShaderModule(device, vk_module, nullptr);
		for (auto l : temp_layouts)
			vkDestroyDescriptorSetLayout(device, l, nullptr);
		spvReflectDestroyShaderModule(&module);
		return nullptr;
	}

	// Bookkeep
	VulkanShader* shader = VersatileResource::allocate<VulkanShader>(resources_allocator);
	shader->shader_module = vk_module;
	shader->stage_create_infos = temp_stage_infos;
	shader->descriptor_set_layouts = temp_layouts;
	shader->push_constant_stages = push_constant_stages;
	shader->pipeline_layout = temp_pipeline_layout;
	shader->vertex_input_variables = temp_vertex_inputs;
	// Reflection Data
	shader->reflection_descriptors = temp_reflection_descriptors;
	shader->reflection_push_constants = temp_reflection_pc;

	// Generate hash
	shader->shader_hash = (size_t)vk_module ^ (size_t)temp_pipeline_layout ^ (size_t)shader_stages;

	spvReflectDestroyShaderModule(&module);
	return (Shader)shader;
}

void VulkanRenderBackend::shader_free(Shader p_shader) {
	VulkanShader* shader_info = (VulkanShader*)p_shader;

	for (size_t i = 0; i < shader_info->descriptor_set_layouts.size(); i++) {
		vkDestroyDescriptorSetLayout(device, shader_info->descriptor_set_layouts[i], nullptr);
	}

	vkDestroyPipelineLayout(device, shader_info->pipeline_layout, nullptr);

	for (size_t i = 0; i < shader_info->stage_create_infos.size(); i++) {
		vkDestroyShaderModule(device, shader_info->stage_create_infos[i].module, nullptr);
	}

	VersatileResource::free(resources_allocator, shader_info);
}

std::vector<ShaderDescriptorReflection> VulkanRenderBackend::shader_get_descriptor_layouts(
		Shader p_shader) {
	if (!p_shader) {
		return {};
	}
	return ((VulkanShader*)p_shader)->reflection_descriptors;
}

std::vector<ShaderPushConstantReflection> VulkanRenderBackend::shader_get_push_constants(
		Shader p_shader) {
	if (!p_shader) {
		return {};
	}
	return ((VulkanShader*)p_shader)->reflection_push_constants;
}

std::vector<ShaderInterfaceVariable> VulkanRenderBackend::shader_get_vertex_inputs(
		Shader p_shader) {
	if (!p_shader) {
		return {};
	}
	return ((VulkanShader*)p_shader)->vertex_input_variables;
}

} //namespace gl