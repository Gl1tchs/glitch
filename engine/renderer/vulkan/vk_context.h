#pragma once

#include <VkBootstrap.h>
#include <vk_mem_alloc.h>

#include "core/deletion_queue.h"

class Window;

struct VulkanContext {
	VkInstance instance;
	VkDebugUtilsMessengerEXT debug_messenger;

	VkSurfaceKHR surface;
	VkPhysicalDevice chosen_gpu;
	VkDevice device;

	VkQueue graphics_queue;
	uint32_t graphics_queue_family;

	VmaAllocator allocator;

	DeletionQueue deletion_queue;

	void init(Ref<Window> window);

	void destroy();
};
