#pragma once

#include "platform/vulkan/vk_common.h"

struct VulkanBuffer {
	VkBuffer buffer;
	VmaAllocation allocation;
	VmaAllocationInfo info;
};

VulkanBuffer vk_create_buffer(VmaAllocator allocator, size_t alloc_size,
		VkBufferUsageFlags usage, VmaMemoryUsage memory_usage);

void vk_destroy_buffer(VmaAllocator allocator, const VulkanBuffer& buffer);
