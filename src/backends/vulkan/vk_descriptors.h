#pragma once

#include "core/templates/vector_view.h"

#include "renderer/types.h"

#include "backends/vulkan/vk_context.h"

#include <vulkan/vulkan.h>

struct VulkanUniformSet {
	VkDescriptorSet vk_descriptor_set = VK_NULL_HANDLE;
	VkDescriptorPool vk_descriptor_pool = VK_NULL_HANDLE;
	DescriptorSetPools::iterator pool_sets_it = {};
};

namespace vk {

static const uint32_t MAX_UNIFORM_POOL_ELEMENT = 65535;

VkDescriptorPool descriptor_set_pool_find_or_create(Context p_context,
		const DescriptorSetPoolKey& p_key,
		DescriptorSetPools::iterator* r_pool_sets_it);

void descriptor_set_pool_unreference(Context p_context,
		DescriptorSetPools::iterator p_pool_sets_it,
		VkDescriptorPool p_vk_descriptor_pool);

UniformSet uniform_set_create(Context p_context,
		VectorView<BoundUniform> p_uniforms, Shader p_shader,
		uint32_t p_set_index);

void uniform_set_free(Context p_context, UniformSet p_uniform_set);

} //namespace vk