#include "platform/vulkan/vk_backend.h"

Buffer VulkanRenderBackend::buffer_create(uint64_t p_size,
		BitField<BufferUsageBits> p_usage,
		MemoryAllocationType p_allocation_type) {
	VkBufferCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	create_info.pNext = nullptr;
	create_info.size = p_size;
	create_info.usage = p_usage;

	VmaAllocationCreateInfo alloc_create_info = {};
	switch (p_allocation_type) {
		case MEMORY_ALLOCATION_TYPE_CPU: {
			bool is_src = p_usage.has_flag(BUFFER_USAGE_TRANSFER_SRC_BIT);
			bool is_dst = p_usage.has_flag(BUFFER_USAGE_TRANSFER_DST_BIT);
			if (is_src && !is_dst) {
				// Looks like a staging buffer: CPU maps, writes sequentially,
				// then GPU copies to VRAM.
				alloc_create_info.flags =
						VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
			}
			if (is_dst && !is_src) {
				// Looks like a readback buffer: GPU copies from VRAM, then CPU
				// maps and reads.
				alloc_create_info.flags =
						VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
			}
			alloc_create_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
			alloc_create_info.requiredFlags =
					(VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
							VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		} break;
		case MEMORY_ALLOCATION_TYPE_GPU: {
			alloc_create_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

			if (p_size <= SMALL_ALLOCATION_MAX_SIZE) {
				uint32_t mem_type_index = 0;
				vmaFindMemoryTypeIndexForBufferInfo(allocator, &create_info,
						&alloc_create_info, &mem_type_index);
				alloc_create_info.pool =
						_find_or_create_small_allocs_pool(mem_type_index);
			}
		} break;
	}

	// allocate the buffer
	VkBuffer vk_buffer = VK_NULL_HANDLE;
	VmaAllocation allocation = nullptr;
	VmaAllocationInfo alloc_info = {};

	VK_CHECK(vmaCreateBuffer(allocator, &create_info, &alloc_create_info,
			&vk_buffer, &allocation, &alloc_info));

	// Bookkeep.
	VulkanBuffer* buf_info =
			VersatileResource::allocate<VulkanBuffer>(resources_allocator);
	buf_info->vk_buffer = vk_buffer;
	buf_info->allocation.handle = allocation;
	buf_info->allocation.size = alloc_info.size;
	buf_info->size = p_size;

	return Buffer(buf_info);
}

void VulkanRenderBackend::buffer_free(Buffer p_buffer) {
	if (!p_buffer) {
		return;
	}

	VulkanBuffer* buffer = (VulkanBuffer*)p_buffer;

	if (buffer->vk_view) {
		vkDestroyBufferView(device, buffer->vk_view, nullptr);
	}
	vmaDestroyBuffer(allocator, buffer->vk_buffer, buffer->allocation.handle);
	VersatileResource::free(resources_allocator, buffer);
}

BufferDeviceAddress VulkanRenderBackend::buffer_get_device_address(
		Buffer p_buffer) {
	VulkanBuffer* buffer = (VulkanBuffer*)p_buffer;

	VkBufferDeviceAddressInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_KHR;
	info.buffer = buffer->vk_buffer;

	return vkGetBufferDeviceAddress(device, &info);
}

uint8_t* VulkanRenderBackend::buffer_map(Buffer p_buffer) {
	VulkanBuffer* buffer = (VulkanBuffer*)p_buffer;

	void* data_ptr = nullptr;
	VK_CHECK(vmaMapMemory(allocator, buffer->allocation.handle, &data_ptr));

	return (uint8_t*)data_ptr;
}

void VulkanRenderBackend::buffer_unmap(Buffer p_buffer) {
	VulkanBuffer* buffer = (VulkanBuffer*)p_buffer;

	vmaUnmapMemory(allocator, buffer->allocation.handle);
}

VmaPool VulkanRenderBackend::_find_or_create_small_allocs_pool(
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
