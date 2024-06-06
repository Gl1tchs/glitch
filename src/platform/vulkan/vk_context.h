#pragma once

#include "core/templates/versatile_resource.h"

#include "platform/vulkan/vk_buffer.h"
#include "platform/vulkan/vk_descriptors.h"
#include "platform/vulkan/vk_image.h"
#include "platform/vulkan/vk_queue.h"
#include "platform/vulkan/vk_shader.h"

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

using VersatileResource = VersatileResourceTemplate<VulkanBuffer, VulkanImage,
		VulkanShader, VulkanUniformSet>;

struct VulkanContext {
	VkInstance instance;
	VkDevice device;

	VkPhysicalDevice physical_device;
	VkPhysicalDeviceProperties physical_device_properties;
	VkPhysicalDeviceFeatures physical_device_features;

	VkSurfaceKHR surface;
	VkDebugUtilsMessengerEXT debug_messenger;

	VulkanQueue graphics_queue;
	VulkanQueue present_queue;
	VulkanQueue command_queue;

	static const uint32_t SMALL_ALLOCATION_MAX_SIZE = 4096;

	VmaAllocator allocator = nullptr;
	std::unordered_map<uint32_t, VmaPool> small_allocs_pools;

	DescriptorSetPools descriptor_set_pools;
	uint32_t max_descriptor_sets_per_pool = 0;

	PagedAllocator<VersatileResource> resources_allocator;

public:
	VmaPool find_or_create_small_allocs_pool(uint32_t p_mem_type_index);

	void immediate_submit(
			std::function<void(CommandBuffer p_cmd)>&& p_function);

private:
	// immediate commands
	Fence imm_fence;
	CommandBuffer imm_command_buffer;
	CommandPool imm_command_pool;
};
