#include "glitch/platform/vulkan/vk_backend.h"

namespace gl {

Fence VulkanRenderBackend::fence_create() {
	VkFenceCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT; // signal on create

	VkFence vk_fence = VK_NULL_HANDLE;
	VK_CHECK(vkCreateFence(device, &create_info, nullptr, &vk_fence));

	return Fence(vk_fence);
}

void VulkanRenderBackend::fence_free(Fence p_fence) {
	vkDestroyFence(device, (VkFence)p_fence, nullptr);
}

void VulkanRenderBackend::fence_wait(Fence p_fence) {
	VK_CHECK(vkWaitForFences(
			device, 1, (VkFence*)&p_fence, VK_TRUE, UINT64_MAX));
}

void VulkanRenderBackend::fence_reset(Fence p_fence) {
	VK_CHECK(vkResetFences(device, 1, (VkFence*)&p_fence));
}

Semaphore VulkanRenderBackend::semaphore_create() {
	VkSemaphoreCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkSemaphore vk_semaphore = VK_NULL_HANDLE;
	VK_CHECK(vkCreateSemaphore(device, &create_info, nullptr, &vk_semaphore));

	return Semaphore(vk_semaphore);
}

void VulkanRenderBackend::semaphore_free(Semaphore p_semaphore) {
	vkDestroySemaphore(device, (VkSemaphore)p_semaphore, nullptr);
}

} //namespace gl