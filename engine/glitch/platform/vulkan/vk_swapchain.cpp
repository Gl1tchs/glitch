#include "glitch/platform/vulkan/vk_backend.h"

#include <VkBootstrap.h>
#include <vulkan/vulkan_core.h>

namespace gl {

void VulkanRenderBackend::_swapchain_release(VulkanSwapchain* p_swapchain) {
	// destroy swapchain resources
	for (int i = 0; i < p_swapchain->images.size(); i++) {
		vkDestroyImageView(device, p_swapchain->images[i].vk_image_view, nullptr);
	}

	p_swapchain->image_index = UINT32_MAX;
	p_swapchain->images.clear();

	vkDestroySwapchainKHR(device, p_swapchain->vk_swapchain, nullptr);
}

Swapchain VulkanRenderBackend::swapchain_create() {
	VulkanSwapchain* swapchain = new VulkanSwapchain();
	swapchain->format = VK_FORMAT_R8G8B8A8_UNORM;
	swapchain->color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	swapchain->initialized = false;

	return Swapchain(swapchain);
}

void VulkanRenderBackend::swapchain_resize(
		CommandQueue p_cmd_queue, Swapchain p_swapchain, glm::uvec2 p_size, bool p_vsync) {
	if (!p_swapchain) {
		GL_LOG_ERROR("[VULKAN] [VulkanRenderBackend::swapchain_resize] Unable to resize a "
					 "swapchain that hasn't created!");
		return;
	}

	VulkanQueue* vk_queue = (VulkanQueue*)p_cmd_queue;
	VulkanSwapchain* swapchain = (VulkanSwapchain*)p_swapchain;

	// Wait for device operations to finish
	vkDeviceWaitIdle(device);

	vkb::SwapchainBuilder swapchain_builder(physical_device, device, surface);
	swapchain_builder =
			swapchain_builder
					.set_desired_format(VkSurfaceFormatKHR{
							.format = swapchain->format,
							.colorSpace = swapchain->color_space,
					})
					// TODO add setting
					.set_desired_present_mode(
							// TODO: i dont think this is the best way
							// to do it
							p_vsync ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_MAILBOX_KHR)
					.set_desired_min_image_count(vkb::SwapchainBuilder::DOUBLE_BUFFERING)
					.set_desired_extent(p_size.x, p_size.y)
					.add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT);

	if (swapchain->initialized) {
		swapchain_builder.set_old_swapchain(swapchain->vk_swapchain);
	}

	vkb::Swapchain vkb_swapchain = swapchain_builder.build().value();

	// Free the old swapchain
	_swapchain_release(swapchain);

	swapchain->vk_swapchain = vkb_swapchain.swapchain;
	swapchain->format = vkb_swapchain.image_format;
	swapchain->color_space = vkb_swapchain.color_space;
	swapchain->extent = vkb_swapchain.extent;

	std::vector<VkImage> swapchain_images = vkb_swapchain.get_images().value();
	std::vector<VkImageView> swapchain_image_views = vkb_swapchain.get_image_views().value();

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

	swapchain->initialized = true;
}

size_t VulkanRenderBackend::swapchain_get_image_count(Swapchain p_swapchain) {
	VulkanSwapchain* swapchain = (VulkanSwapchain*)p_swapchain;
	return swapchain->images.size();
}

std::vector<Image> VulkanRenderBackend::swapchain_get_images(Swapchain p_swapchain) {
	VulkanSwapchain* swapchain = (VulkanSwapchain*)p_swapchain;

	std::vector<Image> images(swapchain->images.size());
	for (size_t i = 0; i < swapchain->images.size(); i++) {
		images[i] = Image(&swapchain->images[i]);
	}

	return images;
}

Result<Image, SwapchainAcquireError> VulkanRenderBackend::swapchain_acquire_image(
		Swapchain p_swapchain, Semaphore p_semaphore, uint32_t* o_image_index) {
	VulkanSwapchain* swapchain = (VulkanSwapchain*)p_swapchain;

	const VkResult res = vkAcquireNextImageKHR(device, swapchain->vk_swapchain, UINT64_MAX,
			(VkSemaphore)p_semaphore, VK_NULL_HANDLE, &swapchain->image_index);

	if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR) {
		return make_err<Image>(SwapchainAcquireError::OUT_OF_DATE);
	} else if (res != VK_SUCCESS) {
		return make_err<Image>(SwapchainAcquireError::ERROR);
	}

	if (o_image_index) {
		*o_image_index = swapchain->image_index;
	}

	return Image(&swapchain->images[swapchain->image_index]);
}

glm::uvec2 VulkanRenderBackend::swapchain_get_extent(Swapchain p_swapchain) {
	GL_ASSERT(p_swapchain != nullptr);

	VulkanSwapchain* swapchain = (VulkanSwapchain*)p_swapchain;

	glm::uvec2 extent;
	static_assert(sizeof(glm::uvec2) == sizeof(VkExtent2D));
	memcpy(&extent, &swapchain->extent, sizeof(VkExtent2D));

	return extent;
}

DataFormat VulkanRenderBackend::swapchain_get_format(Swapchain p_swapchain) {
	GL_ASSERT(p_swapchain != nullptr);

	VulkanSwapchain* swapchain = (VulkanSwapchain*)p_swapchain;
	return static_cast<DataFormat>(swapchain->format);
}

void VulkanRenderBackend::swapchain_free(Swapchain p_swapchain) {
	GL_ASSERT(p_swapchain != nullptr);

	VulkanSwapchain* swapchain = (VulkanSwapchain*)p_swapchain;
	_swapchain_release(swapchain);

	delete swapchain;
}

} //namespace gl