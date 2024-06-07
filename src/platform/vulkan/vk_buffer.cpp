#include "platform/vulkan/vk_buffer.h"

#include "platform/vulkan/vk_common.h"
#include "platform/vulkan/vk_context.h"

namespace vk {

Buffer buffer_create(Context p_context, uint64_t p_size,
		BitField<BufferUsageBits> p_usage,
		MemoryAllocationType p_allocation_type) {
	VulkanContext* context = (VulkanContext*)p_context;

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

			if (p_size <= VulkanContext::SMALL_ALLOCATION_MAX_SIZE) {
				uint32_t mem_type_index = 0;
				vmaFindMemoryTypeIndexForBufferInfo(context->allocator,
						&create_info, &alloc_create_info, &mem_type_index);
				alloc_create_info.pool =
						context->find_or_create_small_allocs_pool(
								mem_type_index);
			}
		} break;
	}

	// allocate the buffer
	VkBuffer vk_buffer = VK_NULL_HANDLE;
	VmaAllocation allocation = nullptr;
	VmaAllocationInfo alloc_info = {};

	VK_CHECK(vmaCreateBuffer(context->allocator, &create_info,
			&alloc_create_info, &vk_buffer, &allocation, &alloc_info));

	// Bookkeep.
	VulkanBuffer* buf_info = VersatileResource::allocate<VulkanBuffer>(
			context->resources_allocator);
	buf_info->vk_buffer = vk_buffer;
	buf_info->allocation.handle = allocation;
	buf_info->allocation.size = alloc_info.size;
	buf_info->size = p_size;

	return Buffer(buf_info);
}

void buffer_free(Context p_context, Buffer p_buffer) {
	if (!p_buffer) {
		return;
	}

	VulkanContext* context = (VulkanContext*)p_context;
	VulkanBuffer* buffer = (VulkanBuffer*)p_buffer;

	if (buffer->vk_view) {
		vkDestroyBufferView(context->device, buffer->vk_view, nullptr);
	}
	vmaDestroyBuffer(
			context->allocator, buffer->vk_buffer, buffer->allocation.handle);
	VersatileResource::free(context->resources_allocator, buffer);
}

uint8_t* buffer_map(Context p_context, Buffer p_buffer) {
	const VulkanContext* context = (VulkanContext*)p_context;
	VulkanBuffer* buffer = (VulkanBuffer*)p_buffer;

	void* data_ptr = nullptr;
	VK_CHECK(vmaMapMemory(
			context->allocator, buffer->allocation.handle, &data_ptr));

	return (uint8_t*)data_ptr;
}

void buffer_unmap(Context p_context, Buffer p_buffer) {
	const VulkanContext* context = (VulkanContext*)p_context;
	VulkanBuffer* buffer = (VulkanBuffer*)p_buffer;

	vmaUnmapMemory(context->allocator, buffer->allocation.handle);
}

} //namespace vk
