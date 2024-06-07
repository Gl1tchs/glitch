#pragma once

#include "renderer/render_backend.h"

#include <vulkan/vulkan.h>

struct VulkanShader {
	std::vector<VkPipelineShaderStageCreateInfo> stage_create_infos;
	std::vector<VkDescriptorSetLayout> descriptor_set_layouts;
	VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
};

namespace vk {

Shader shader_create_from_bytecode(
		Context p_context, const std::vector<SpirvData>& p_shaders);

void shader_free(Context p_context, Shader p_shader);

} //namespace vk
