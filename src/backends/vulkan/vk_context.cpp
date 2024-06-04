#include "backends/vulkan/vk_context.h"

#include "backends/vulkan/vk_common.h"

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
