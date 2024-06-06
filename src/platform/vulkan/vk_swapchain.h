#pragma once

#include "renderer/types.h"

#include "platform/vulkan/vk_image.h"

struct VulkanSwapchain {
	VkSwapchainKHR vk_swapchain = VK_NULL_HANDLE;
	VkFormat format = VK_FORMAT_UNDEFINED;
	VkColorSpaceKHR color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	VkExtent2D extent;
	std::vector<VulkanImage> images;
	uint32_t image_index;
};

namespace vk {

// not valid until resized
Swapchain swapchain_create();

void swapchain_resize(Context p_context, CommandQueue p_cmd_queue,
		Swapchain p_swapchain, Vec2u size);

/**
 * @returns `Image` if succeed `{}` if resize needed
 */
Optional<Image> swapchain_acquire_image(
		Context p_context, Swapchain p_swapchain, Semaphore p_semaphore);

Vec2u swapchain_get_extent(Swapchain p_swapchain);

DataFormat swapchain_get_format(Swapchain p_swapchain);

void swapchain_free(Context p_context, Swapchain p_swapchain);

} //namespace vk
