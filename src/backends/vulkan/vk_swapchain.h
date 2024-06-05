#pragma once

#include "renderer/types.h"

#include "backends/vulkan/vk_common.h"

struct VulkanSwapchain {
	VkSwapchainKHR vk_swapchain = VK_NULL_HANDLE;
	VkFormat format = VK_FORMAT_UNDEFINED;
	VkColorSpaceKHR color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	std::vector<VkImage> images;
	std::vector<VkImageView> image_views;
	uint32_t image_index = 0;
};

namespace vk {

Swapchain swap_chain_create(Context p_context);

void swap_chain_resize(Context p_context, CommandQueue p_cmd_queue,
		Swapchain p_swapchain, Vec2u size);

uint32_t swap_chain_acquire_image(
		Context p_context, Swapchain p_swapchain, Semaphore p_semaphore);

DataFormat swap_chain_get_format(Swapchain p_swapchain);

void swap_chain_free(Context p_context, Swapchain p_swapchain);

} //namespace vk
