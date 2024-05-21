#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include "gl/renderer/image.h"

#include "platform/vulkan/vk_descriptors.h"

struct VulkanContext {
	VkInstance instance;
	VkDevice device;
	VkPhysicalDevice chosen_gpu;
	VkSurfaceKHR surface;
	VmaAllocator allocator;
	VkDebugUtilsMessengerEXT debug_messenger;

	VulkanDescriptorAllocator descriptor_allocator;
	VkDescriptorSetLayout scene_data_descriptor_layout;

	VkQueue graphics_queue;
	uint32_t graphics_queue_family{ UINT32_MAX };

	VkQueue present_queue;
	uint32_t present_queue_family{ UINT32_MAX };

	VkFormat color_attachment_format;
	VkFormat depth_attachment_format;

	VkSampler linear_sampler;
	VkSampler nearest_sampler;

	VkSampler get_sampler(ImageFilteringMode mode) const;
};
