#include "glitch/platform/vulkan/vk_backend.h"

namespace gl {

CommandQueue VulkanRenderBackend::queue_get(QueueType p_type) {
	VulkanQueue* queue;
	switch (p_type) {
		case QueueType::GRAPHICS:
			queue = &graphics_queue;
			break;
		case QueueType::PRESENT:
			queue = &present_queue;
			break;
		case QueueType::TRANSFER:
			queue = &transfer_queue;
			break;
		default:
			queue = &graphics_queue;
			break;
	}

	return CommandQueue(queue);
}

void VulkanRenderBackend::queue_submit(CommandQueue p_queue,
		CommandBuffer p_cmd, Fence p_fence, Semaphore p_wait_semaphore,
		Semaphore p_signal_semaphore) {
	VkCommandBufferSubmitInfo cmd_info = {};
	cmd_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
	cmd_info.pNext = nullptr;
	cmd_info.commandBuffer = (VkCommandBuffer)p_cmd;
	cmd_info.deviceMask = 0;

	VkSemaphoreSubmitInfo wait_semaphore_info = {};
	wait_semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
	wait_semaphore_info.semaphore = (VkSemaphore)p_wait_semaphore;
	wait_semaphore_info.stageMask =
			VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT; // TODO get as
															 // parameter
	wait_semaphore_info.deviceIndex = 0;
	wait_semaphore_info.value = 1;

	VkSemaphoreSubmitInfo signal_semaphore_info = {};
	signal_semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
	signal_semaphore_info.semaphore = (VkSemaphore)p_signal_semaphore;
	signal_semaphore_info.stageMask =
			VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT; // TODO get as parameter
	signal_semaphore_info.deviceIndex = 0;
	signal_semaphore_info.value = 1;

	VkSubmitInfo2 submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
	submit_info.pNext = nullptr;

	submit_info.commandBufferInfoCount = 1;
	submit_info.pCommandBufferInfos = &cmd_info;

	submit_info.waitSemaphoreInfoCount = p_wait_semaphore == nullptr ? 0 : 1;
	submit_info.pWaitSemaphoreInfos = &wait_semaphore_info;

	submit_info.signalSemaphoreInfoCount =
			p_signal_semaphore == nullptr ? 0 : 1;
	submit_info.pSignalSemaphoreInfos = &signal_semaphore_info;

	VulkanQueue* queue = (VulkanQueue*)p_queue;

	// Lock queue for thread safe access
	std::lock_guard<std::mutex> lock(queue->mutex);

	VK_CHECK(vkQueueSubmit2(queue->queue, 1, &submit_info, (VkFence)p_fence));
}

bool VulkanRenderBackend::queue_present(CommandQueue p_queue,
		Swapchain p_swapchain, Semaphore p_wait_semaphore) {
	VulkanSwapchain* swapchain = (VulkanSwapchain*)p_swapchain;
	VulkanQueue* queue = (VulkanQueue*)p_queue;

	VkPresentInfoKHR present_info = {};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = p_wait_semaphore == nullptr ? 0 : 1;
	present_info.pWaitSemaphores = (VkSemaphore*)&p_wait_semaphore;
	present_info.swapchainCount = 1;
	present_info.pSwapchains = &swapchain->vk_swapchain;
	present_info.pImageIndices = &swapchain->image_index;

	// Lock queue for thread safe access
	std::lock_guard<std::mutex> lock(queue->mutex);

	const VkResult res = vkQueuePresentKHR(queue->queue, &present_info);
	return res == VK_SUCCESS;
}

} //namespace gl