#pragma once

#include "platform/vulkan/vk_common.h"

class VulkanSwapchain {
public:
	VulkanSwapchain(VkDevice device, VkPhysicalDevice chosen_gpu,
			VkSurfaceKHR surface, Vec2u size);

	~VulkanSwapchain();

	VkResult request_next_image(VkSemaphore semaphore, uint32_t* image_index);

	void resize(Vec2u size);

	VkImage get_image(uint32_t image_index) {
		return swapchain_images[image_index];
	}

	VkImageView get_image_view(uint32_t image_index) {
		return swapchain_image_views[image_index];
	}

	VkSwapchainKHR* get_swapchain() { return &swapchain; }

	VkExtent2D get_extent() { return swapchain_extent; }

private:
	void _create(Vec2u size);

	void _destroy();

private:
	VkDevice device;
	VkPhysicalDevice chosen_gpu;
	VkSurfaceKHR surface;

	VkSwapchainKHR swapchain;
	VkFormat swapchain_format;

	std::vector<VkImage> swapchain_images;
	std::vector<VkImageView> swapchain_image_views;
	VkExtent2D swapchain_extent;
};
