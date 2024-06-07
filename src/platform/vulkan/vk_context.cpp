#include "platform/vulkan/vk_context.h"

#include "platform/vulkan/vk_commands.h"
#include "platform/vulkan/vk_common.h"

VmaPool VulkanContext::find_or_create_small_allocs_pool(
		uint32_t p_mem_type_index) {
	if (small_allocs_pools.find(p_mem_type_index) != small_allocs_pools.end()) {
		return small_allocs_pools[p_mem_type_index];
	}

	VmaPoolCreateInfo pci = {};
	pci.memoryTypeIndex = p_mem_type_index;
	pci.flags = 0;
	pci.blockSize = 0;
	pci.minBlockCount = 0;
	pci.maxBlockCount = SIZE_MAX;
	pci.priority = 0.5f;
	pci.minAllocationAlignment = 0;
	pci.pMemoryAllocateNext = nullptr;

	VmaPool pool = VK_NULL_HANDLE;
	VK_CHECK(vmaCreatePool(allocator, &pci, &pool));

	small_allocs_pools[p_mem_type_index] =
			pool; // Don't try to create it again if failed the first time.

	return pool;
}

namespace vk {

void immediate_submit(Context p_context,
		std::function<void(CommandBuffer p_cmd)>&& p_function) {
	VulkanContext* context = (VulkanContext*)p_context;

	VK_CHECK(vkResetFences(context->device, 1, (VkFence*)&context->imm_fence));

	vk::command_reset(context->imm_command_buffer);

	vk::command_begin(context->imm_command_buffer);
	{
		// run the command
		p_function(context->imm_command_buffer);
	}
	vk::command_end(context->imm_command_buffer);

	// submit command buffer to the queue and execute it.
	// context->imm_fence will now block until the graphic commands finish
	// execution
	vk::queue_submit((CommandQueue)&context->graphics_queue,
			context->imm_command_buffer, context->imm_fence);

	// wait till the operation finishes
	VK_CHECK(vkWaitForFences(context->device, 1, (VkFence*)&context->imm_fence,
			true, UINT64_MAX));
}

} //namespace vk
