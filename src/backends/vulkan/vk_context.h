#pragma once

#include "core/templates/versatile_resource.h"

#include "backends/vulkan/vk_buffer.h"
#include "backends/vulkan/vk_shader.h"

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

using VersatileResource = VersatileResourceTemplate<VulkanBuffer,
		/*VulkanImage,*/ VulkanShader
		/*, VulkanDescriptor */>;

struct Queue {
	VkQueue queue;
	uint32_t queue_family;
};

struct VulkanContext {
	VkInstance instance;
	VkDevice device;

	VkPhysicalDevice physical_device;
	VkPhysicalDeviceProperties physical_device_properties;
	VkPhysicalDeviceFeatures physical_device_features;

	VkSurfaceKHR surface;
	VkDebugUtilsMessengerEXT debug_messenger;

	Queue graphics_queue;
	Queue present_queue;
	Queue command_queue;

	VmaAllocator allocator = nullptr;
	std::unordered_map<uint32_t, VmaPool> small_allocs_pools;

	static const uint32_t SMALL_ALLOCATION_MAX_SIZE = 4096;

	PagedAllocator<VersatileResource> resources_allocator;

	VmaPool find_or_create_small_allocs_pool(uint32_t p_mem_type_index);
};
