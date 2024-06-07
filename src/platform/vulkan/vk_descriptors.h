#pragma once

#include "core/templates/vector_view.h"

#include "renderer/types.h"

#include <vulkan/vulkan.h>

struct DescriptorSetPoolKey {
	uint16_t uniform_type[UNIFORM_TYPE_MAX] = {};

	bool operator<(const DescriptorSetPoolKey& p_other) const {
		return memcmp(uniform_type, p_other.uniform_type,
					   sizeof(uniform_type)) < 0;
	}
};

using DescriptorSetPools = std::map<DescriptorSetPoolKey,
		std::unordered_map<VkDescriptorPool, uint32_t>>;

struct VulkanUniformSet {
	VkDescriptorSet vk_descriptor_set = VK_NULL_HANDLE;
	VkDescriptorPool vk_descriptor_pool = VK_NULL_HANDLE;
	DescriptorSetPools::iterator pool_sets_it = {};
};

namespace vk {

static const uint32_t MAX_UNIFORM_POOL_ELEMENT = 65535;

UniformPool uniform_pool_find_or_create(Context p_context,
		const DescriptorSetPoolKey& p_key,
		DescriptorSetPools::iterator* r_pool_sets_it);

void uniform_pool_unreference(Context p_context,
		DescriptorSetPools::iterator p_pool_sets_it,
		VkDescriptorPool p_vk_descriptor_pool);

UniformSet uniform_set_create(Context p_context,
		VectorView<BoundUniform> p_uniforms, Shader p_shader,
		uint32_t p_set_index);

void uniform_set_free(Context p_context, UniformSet p_uniform_set);

} //namespace vk
