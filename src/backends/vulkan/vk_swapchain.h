#pragma once

#include "backends/vulkan/vk_common.h"

struct VulkanSwapchain {
	VkSwapchainKHR swapchain;
	VkExtent2D swapchain_extent;
	VkFormat swapchain_format;

	std::vector<VkImage> swapchain_images;
	std::vector<VkImageView> swapchain_image_views;

	VkResult request_next_image(const VulkanContext& context,
			VkSemaphore semaphore, uint32_t* image_index);

	void resize(const VulkanContext& context, Vec2u size);

	VkImage get_image(uint32_t image_index) {
		return swapchain_images[image_index];
	}

	VkImageView get_image_view(uint32_t image_index) {
		return swapchain_image_views[image_index];
	}

	VkSwapchainKHR* get_swapchain() { return &swapchain; }

	VkExtent2D get_extent() { return swapchain_extent; }

	static Ref<VulkanSwapchain> create(
			const VulkanContext& context, Vec2u size);

	static void create(const VulkanContext& context, Vec2u size,
			VulkanSwapchain* out_swapchain,
			VkSwapchainKHR old_swapchain = VK_NULL_HANDLE);

	static void destroy(
			const VulkanContext& context, VulkanSwapchain* swapchain);
};
