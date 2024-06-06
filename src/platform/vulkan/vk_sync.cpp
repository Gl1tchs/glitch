#include "platform/vulkan/vk_sync.h"

#include "platform/vulkan/vk_common.h"
#include "platform/vulkan/vk_context.h"

#include <vulkan/vulkan_core.h>

namespace vk {

Fence fence_create(Context p_context) {
	VulkanContext* context = (VulkanContext*)p_context;

	VkFenceCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT; // signal on create

	VkFence vk_fence = VK_NULL_HANDLE;
	VK_CHECK(vkCreateFence(context->device, &create_info, nullptr, &vk_fence));

	return Fence(vk_fence);
}

void fence_free(Context p_context, Fence p_fence) {
	VulkanContext* context = (VulkanContext*)p_context;

	vkDestroyFence(context->device, (VkFence)p_fence, nullptr);
}

void fence_wait(Context p_context, Fence p_fence) {
	VulkanContext* context = (VulkanContext*)p_context;
	VK_CHECK(vkWaitForFences(
			context->device, 1, (VkFence*)&p_fence, VK_TRUE, UINT64_MAX));
}

void fence_reset(Context p_context, Fence p_fence) {
	VulkanContext* context = (VulkanContext*)p_context;
	VK_CHECK(vkResetFences(context->device, 1, (VkFence*)&p_fence));
}

Semaphore semaphore_create(Context p_context) {
	VulkanContext* context = (VulkanContext*)p_context;

	VkSemaphoreCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkSemaphore vk_semaphore = VK_NULL_HANDLE;
	VK_CHECK(vkCreateSemaphore(
			context->device, &create_info, nullptr, &vk_semaphore));

	return Semaphore(vk_semaphore);
}

void semaphore_free(Context p_context, Semaphore p_semaphore) {
	VulkanContext* context = (VulkanContext*)p_context;

	vkDestroySemaphore(context->device, (VkSemaphore)p_semaphore, nullptr);
}

} //namespace vk
