#include "platform/vulkan/vk_buffer.h"

VulkanBuffer VulkanBuffer::create(VmaAllocator allocator, size_t alloc_size,
		VkBufferUsageFlags usage, VmaMemoryUsage memory_usage) {
	VkBufferCreateInfo info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.pNext = nullptr,
		.size = alloc_size,
		.usage = usage,
	};

	VmaAllocationCreateInfo alloc_info = {
		.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
		.usage = memory_usage,
	};

	// allocate the buffer
	VulkanBuffer buffer{};
	VK_CHECK(vmaCreateBuffer(allocator, &info, &alloc_info, &buffer.buffer,
			&buffer.allocation, &buffer.info));

	return buffer;
}

void VulkanBuffer::destroy(VmaAllocator allocator, const VulkanBuffer& buffer) {
	vmaDestroyBuffer(allocator, buffer.buffer, buffer.allocation);
}
