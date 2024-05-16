#pragma once

#include "platform/vulkan/vk_common.h"

struct VulkanBuffer {
	VkBuffer buffer;
	VmaAllocation allocation;
	VmaAllocationInfo info;

	static VulkanBuffer create(VmaAllocator allocator, size_t alloc_size,
			VkBufferUsageFlags usage, VmaMemoryUsage memory_usage);

	static void destroy(VmaAllocator allocator, const VulkanBuffer& buffer);
};
