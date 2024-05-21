#include "platform/vulkan/vk_swapchain.h"

#include <VkBootstrap.h>

VkResult VulkanSwapchain::request_next_image(const VulkanContext& context,
		VkSemaphore semaphore, uint32_t* image_index) {
	uint32_t swapchain_image_index;
	return vkAcquireNextImageKHR(
			context.device, swapchain, 10000, semaphore, nullptr, image_index);
}

void VulkanSwapchain::resize(const VulkanContext& context, Vec2u size) {
	vkDeviceWaitIdle(context.device);

	destroy(context, this);
	create(context, size, this);
}

Ref<VulkanSwapchain> VulkanSwapchain::create(
		const VulkanContext& context, Vec2u size) {
	Ref<VulkanSwapchain> swapchain = create_ref<VulkanSwapchain>();
	create(context, size, swapchain.get());

	return swapchain;
}

void VulkanSwapchain::create(const VulkanContext& context, Vec2u size,
		VulkanSwapchain* out_swapchain) {
	vkb::SwapchainBuilder swapchain_builder{ context.chosen_gpu, context.device,
		context.surface };

	out_swapchain->swapchain_format = VK_FORMAT_B8G8R8A8_UNORM;

	vkb::Swapchain vkb_swapchain =
			swapchain_builder
					.set_desired_format(VkSurfaceFormatKHR{
							.format = out_swapchain->swapchain_format,
							.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
					.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
					.set_desired_extent(size.x, size.y)
					.add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
					.build()
					.value();

	out_swapchain->swapchain_extent = vkb_swapchain.extent;

	out_swapchain->swapchain = vkb_swapchain.swapchain;
	out_swapchain->swapchain_images = vkb_swapchain.get_images().value();
	out_swapchain->swapchain_image_views =
			vkb_swapchain.get_image_views().value();
}

void VulkanSwapchain::destroy(
		const VulkanContext& context, VulkanSwapchain* swapchain) {
	vkDestroySwapchainKHR(context.device, swapchain->swapchain, nullptr);

	// destroy swapchain resources
	for (int i = 0; i < swapchain->swapchain_image_views.size(); i++) {
		vkDestroyImageView(
				context.device, swapchain->swapchain_image_views[i], nullptr);
	}
}
