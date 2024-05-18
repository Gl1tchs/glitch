#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

struct VulkanContext {
	VkInstance instance;
	VkDevice device;
	VkPhysicalDevice chosen_gpu;
	VkSurfaceKHR surface;
	VmaAllocator allocator;
	VkDebugUtilsMessengerEXT debug_messenger;

	VkQueue graphics_queue;
	uint32_t graphics_queue_family;
};
