#include "platform/vulkan/vk_swapchain.h"

#include "platform/vulkan/vk_common.h"
#include "platform/vulkan/vk_context.h"

#include <VkBootstrap.h>
#include <vulkan/vulkan_core.h>

namespace vk {

static void _swapchain_release(
		VulkanContext* p_context, VulkanSwapchain* p_swapchain) {
	// destroy swapchain resources
	for (int i = 0; i < p_swapchain->images.size(); i++) {
		vkDestroyImageView(p_context->device,
				p_swapchain->images[i].vk_image_view, nullptr);
	}

	p_swapchain->image_index = UINT32_MAX;
	p_swapchain->images.clear();

	vkDestroySwapchainKHR(
			p_context->device, p_swapchain->vk_swapchain, nullptr);
}

Swapchain swapchain_create() {
	VulkanSwapchain* swapchain = new VulkanSwapchain();
	swapchain->format = VK_FORMAT_R8G8B8A8_UNORM;
	swapchain->color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

	return Swapchain(swapchain);
}

void swapchain_resize(Context p_context, CommandQueue p_cmd_queue,
		Swapchain p_swapchain, Vec2u size) {
	VulkanContext* context = (VulkanContext*)p_context;
	VulkanQueue* queue = (VulkanQueue*)p_cmd_queue;
	VulkanSwapchain* swapchain = (VulkanSwapchain*)p_swapchain;

	vkDeviceWaitIdle(context->device);

	_swapchain_release(context, swapchain);

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
	swapchain->format = vkb_swapchain.image_format;
	swapchain->color_space = vkb_swapchain.color_space;
	swapchain->extent = vkb_swapchain.extent;

	std::vector<VkImage> swapchain_images = vkb_swapchain.get_images().value();
	std::vector<VkImageView> swapchain_image_views =
			vkb_swapchain.get_image_views().value();

	for (size_t i = 0; i < swapchain_images.size(); i++) {
		VulkanImage image = {};
		image.vk_image = swapchain_images[i];
		image.vk_image_view = swapchain_image_views[i];
		image.image_format = vkb_swapchain.image_format;
		image.image_extent = VkExtent3D{
			vkb_swapchain.extent.width,
			vkb_swapchain.extent.height,
			1,
		};

		swapchain->images.push_back(image);
	}
}

Optional<Image> swapchain_acquire_image(
		Context p_context, Swapchain p_swapchain, Semaphore p_semaphore) {
	VulkanContext* context = (VulkanContext*)p_context;
	VulkanSwapchain* swapchain = (VulkanSwapchain*)p_swapchain;

	VkResult res = vkAcquireNextImageKHR(context->device,
			swapchain->vk_swapchain, UINT64_MAX, (VkSemaphore)p_semaphore,
			VK_NULL_HANDLE, &swapchain->image_index);

	if (res != VK_SUCCESS) {
		return {};
	}

	return Image(&swapchain->images[swapchain->image_index]);
}

Vec2u swapchain_get_extent(Swapchain p_swapchain) {
	GL_ASSERT(p_swapchain != nullptr);

	VulkanSwapchain* swapchain = (VulkanSwapchain*)p_swapchain;

	Vec2u extent;
	static_assert(sizeof(Vec2u) == sizeof(VkExtent2D));
	memcpy(&extent, &swapchain->extent, sizeof(VkExtent2D));

	return extent;
}

DataFormat swapchain_get_format(Swapchain p_swapchain) {
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

void swapchain_free(Context p_context, Swapchain p_swapchain) {
	GL_ASSERT(p_swapchain != nullptr);

	VulkanContext* context = (VulkanContext*)p_context;

	VulkanSwapchain* swapchain = (VulkanSwapchain*)p_swapchain;
	_swapchain_release(context, swapchain);

	delete swapchain;
}

} //namespace vk
