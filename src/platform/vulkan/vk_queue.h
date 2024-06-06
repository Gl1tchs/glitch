#pragma once

#include "renderer/types.h"

#include "platform/vulkan/vk_common.h"

struct VulkanQueue {
	VkQueue queue;
	uint32_t queue_family;
};

namespace vk {

void queue_submit(CommandQueue p_queue, CommandBuffer p_cmd,
		Fence p_fence = nullptr, Semaphore p_wait_semaphore = nullptr,
		Semaphore p_signal_semaphore = nullptr);

// returns `true` if succeed `false` if resize needed
bool queue_present(Context p_context, CommandQueue p_queue,
		Swapchain p_swapchain, Semaphore p_wait_semaphore = nullptr);

} //namespace vk
