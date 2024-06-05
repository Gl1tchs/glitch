#include "backends/vulkan/vk_swapchain.h"

#include "backends/vulkan/vk_context.h"

#include <VkBootstrap.h>
#include <vulkan/vulkan_core.h>

namespace vk {

static void _swap_chain_release(
		VulkanContext* p_context, VulkanSwapchain* p_swapchain) {
	// destroy swapchain resources
	for (int i = 0; i < p_swapchain->image_views.size(); i++) {
		vkDestroyImageView(
				p_context->device, p_swapchain->image_views[i], nullptr);
	}

	p_swapchain->image_index = UINT32_MAX;
	p_swapchain->images.clear();
	p_swapchain->image_views.clear();

	vkDestroySwapchainKHR(
			p_context->device, p_swapchain->vk_swapchain, nullptr);
}

Swapchain swap_chain_create() {
	VulkanSwapchain* swapchain = new VulkanSwapchain();
	swapchain->format = VK_FORMAT_R8G8B8A8_UNORM;
	swapchain->color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

	return Swapchain(swapchain);
}

void swap_chain_resize(Context p_context, CommandQueue p_cmd_queue,
		Swapchain p_swapchain, Vec2u size) {
	VulkanContext* context = (VulkanContext*)p_context;
	VulkanQueue* queue = (VulkanQueue*)p_cmd_queue;
	VulkanSwapchain* swapchain = (VulkanSwapchain*)p_swapchain;

	vkDeviceWaitIdle(context->device);

	_swap_chain_release(context, swapchain);

	vkb::SwapchainBuilder swapchain_builder{ context->physical_device,
		context->device, context->surface };

	vkb::Swapchain vkb_swapchain =
			swapchain_builder
					.set_desired_format(VkSurfaceFormatKHR{
							.format = swapchain->format,
							.colorSpace = swapchain->color_space,
					})
					// TODO add setting
					.set_desired_present_mode(VK_PRESENT_MODE_IMMEDIATE_KHR)
					.set_desired_min_image_count(
							vkb::SwapchainBuilder::DOUBLE_BUFFERING)
					.set_desired_extent(size.x, size.y)
					.add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
					.build()
					.value();

	swapchain->vk_swapchain = vkb_swapchain.swapchain;
	swapchain->images = vkb_swapchain.get_images().value();
	swapchain->image_views = vkb_swapchain.get_image_views().value();
}

uint32_t swap_chain_acquire_image(
		Context p_context, Swapchain p_swapchain, Semaphore p_semaphore) {
	VulkanContext* context = (VulkanContext*)p_context;
	VulkanSwapchain* swapchain = (VulkanSwapchain*)p_swapchain;
	VkSemaphore semaphore = (VkSemaphore)p_semaphore;

	vkAcquireNextImageKHR(context->device, swapchain->vk_swapchain, UINT64_MAX,
			semaphore, VK_NULL_HANDLE, &swapchain->image_index);

	return swapchain->image_index;
}

DataFormat swap_chain_get_format(Swapchain p_swapchain) {
	GL_ASSERT(p_swapchain != nullptr);

	VulkanSwapchain* swapchain = (VulkanSwapchain*)p_swapchain;
	switch (swapchain->format) {
		case VK_FORMAT_B8G8R8A8_UNORM:
			return DATA_FORMAT_B8G8R8A8_UNORM;
		case VK_FORMAT_R8G8B8A8_UNORM:
			return DATA_FORMAT_R8G8B8A8_UNORM;
		default:
			GL_ASSERT(false, "Unknown swapchain format.");
			return DATA_FORMAT_MAX;
	}
}

void swap_chain_free(Context p_context, Swapchain p_swapchain) {
	GL_ASSERT(p_swapchain != nullptr);

	VulkanContext* context = (VulkanContext*)p_context;

	VulkanSwapchain* swapchain = (VulkanSwapchain*)p_swapchain;
	_swap_chain_release(context, swapchain);

	delete swapchain;
}

} //namespace vk
