#include "platform/vulkan/vk_swapchain.h"

#include <VkBootstrap.h>

VulkanSwapchain::VulkanSwapchain(VkDevice device, VkPhysicalDevice chosen_gpu,
		VkSurfaceKHR surface, Vector2u size) :
		device(device),
		chosen_gpu(chosen_gpu),
		surface(surface),
		swapchain(VK_NULL_HANDLE),
		swapchain_format(VK_FORMAT_R16G16B16A16_SFLOAT),
		swapchain_extent(VkExtent2D{ size.x, size.y }) {
    _create(size);
}

VulkanSwapchain::~VulkanSwapchain() { _destroy(); }

VkResult VulkanSwapchain::request_next_image(
		VkSemaphore semaphore, uint32_t* image_index) {
	uint32_t swapchain_image_index;
	return vkAcquireNextImageKHR(
			device, swapchain, 10000, semaphore, nullptr, image_index);
}

void VulkanSwapchain::resize(Vector2u size) {
	vkDeviceWaitIdle(device);

	_destroy();

	_create(size);
}

void VulkanSwapchain::_create(Vector2u size) {
	vkb::SwapchainBuilder swapchain_builder{ chosen_gpu, device, surface };

	swapchain_format = VK_FORMAT_B8G8R8A8_UNORM;

	vkb::Swapchain vkb_swapchain =
			swapchain_builder
					.set_desired_format(VkSurfaceFormatKHR{
							.format = swapchain_format,
							.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
					.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
					.set_desired_extent(size.x, size.y)
					.add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
					.build()
					.value();

	swapchain_extent = vkb_swapchain.extent;

	swapchain = vkb_swapchain.swapchain;
	swapchain_images = vkb_swapchain.get_images().value();
	swapchain_image_views = vkb_swapchain.get_image_views().value();
}

void VulkanSwapchain::_destroy() {
	vkDestroySwapchainKHR(device, swapchain, nullptr);

	// destroy swapchain resources
	for (int i = 0; i < swapchain_image_views.size(); i++) {
		vkDestroyImageView(device, swapchain_image_views[i], nullptr);
	}
}
