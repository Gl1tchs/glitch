#pragma once

#include "renderer/vulkan/vk_pch.h"

class VulkanSwapchain {
public:
	VulkanSwapchain(VkDevice device, VkPhysicalDevice chosen_gpu,
			VkSurfaceKHR surface, Vector2u size);

	~VulkanSwapchain();

	VkResult request_next_image(VkSemaphore semaphore, uint32_t* image_index);

	void resize(Vector2u size);

private:
	void _create(Vector2u size);

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
