#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include "core/deletion_queue.h"
#include "renderer/image.h"

#include "platform/vulkan/vk_descriptors.h"

struct VulkanContext {
	VkInstance instance;
	VkDevice device;
	VkPhysicalDevice chosen_gpu;
	VkSurfaceKHR surface;
	VmaAllocator allocator;
	VkDebugUtilsMessengerEXT debug_messenger;

	VulkanDescriptorAllocator descriptor_allocator;

	DeletionQueue deletion_queue;

	VkQueue graphics_queue;
	uint32_t graphics_queue_family;

	VkFormat color_attachment_format;
	VkFormat depth_attachment_format;

	VkSampler linear_sampler;
	VkSampler nearest_sampler;

	VkSampler get_sampler(ImageFilteringMode mode) const;
};
