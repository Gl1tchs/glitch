#include "platform/vulkan/vk_swapchain.h"

#include <VkBootstrap.h>
#include <vulkan/vulkan_core.h>

VkResult VulkanSwapchain::request_next_image(const VulkanContext& context,
		VkSemaphore semaphore, uint32_t* image_index) {
	uint32_t swapchain_image_index;
	return vkAcquireNextImageKHR(context.device, swapchain, UINT64_MAX,
			semaphore, VK_NULL_HANDLE, image_index);
}

void VulkanSwapchain::resize(const VulkanContext& context, Vec2u size) {
	vkDeviceWaitIdle(context.device);

	// create new swapchain with the old reference
	VulkanSwapchain new_swapchain;
	create(context, size, &new_swapchain, swapchain);

	// destroy the old swapchain
	destroy(context, this);

	// make new swapchain
	*this = new_swapchain;
}

Ref<VulkanSwapchain> VulkanSwapchain::create(
		const VulkanContext& context, Vec2u size) {
	Ref<VulkanSwapchain> swapchain = create_ref<VulkanSwapchain>();
	create(context, size, swapchain.get());

	return swapchain;
}

void VulkanSwapchain::create(const VulkanContext& context, Vec2u size,
		VulkanSwapchain* out_swapchain, VkSwapchainKHR old_swapchain) {
	vkb::SwapchainBuilder swapchain_builder{ context.chosen_gpu, context.device,
		context.surface };

	// for faster recreation
	if (old_swapchain != VK_NULL_HANDLE) {
		swapchain_builder.set_old_swapchain(old_swapchain);
	}

	vkb::Swapchain vkb_swapchain =
			swapchain_builder
					.set_desired_format(VkSurfaceFormatKHR{
							.format = VK_FORMAT_B8G8R8A8_UNORM,
							.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
					})
                    // TODO add setting
					.set_desired_present_mode(VK_PRESENT_MODE_IMMEDIATE_KHR)
					.set_desired_min_image_count(
							vkb::SwapchainBuilder::DOUBLE_BUFFERING)
					.set_desired_extent(size.x, size.y)
					.add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
					.build()
					.value();

	out_swapchain->swapchain_extent = vkb_swapchain.extent;
	out_swapchain->swapchain_format = vkb_swapchain.image_format;

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
