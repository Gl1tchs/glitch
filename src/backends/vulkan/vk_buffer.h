#pragma once

#include "core/templates/bit_field.h"
#include "renderer/render_backend.h"
#include "renderer/types.h"

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

struct VulkanBuffer {
	VkBuffer vk_buffer;
	struct {
		VmaAllocation handle;
		uint64_t size = UINT64_MAX;
	} allocation;
	uint64_t size = 0;
	VkBufferView vk_view = VK_NULL_HANDLE;
};

namespace vk {

Buffer buffer_create(Context p_context, uint64_t p_size,
		BitField<BufferUsageBits> p_usage,
		MemoryAllocationType p_allocation_type);

void buffer_free(Context p_context, Buffer p_buffer);

uint8_t* buffer_map(Context p_context, Buffer p_buffer);

void buffer_unmap(Context p_context, Buffer p_buffer);

} //namespace vk
