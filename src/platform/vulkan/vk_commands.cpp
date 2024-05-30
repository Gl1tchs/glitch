#include "platform/vulkan/vk_commands.h"

#include "platform/vulkan/vk_init.h"

#include <vulkan/vulkan_core.h>

VulkanCommandPool VulkanCommandPool::create(
		VkDevice device, const VkCommandPoolCreateInfo* info) {
	VulkanCommandPool command_pool{ VK_NULL_HANDLE };
	VK_CHECK(vkCreateCommandPool(
			device, info, nullptr, &command_pool.command_pool));

	return command_pool;
}

VulkanCommandPool VulkanCommandPool::create(VkDevice device,
		uint32_t queue_family_index, VkCommandPoolCreateFlags flags) {
	VkCommandPoolCreateInfo pool_info =
			vkinit::command_pool_create_info(queue_family_index, flags);

	return VulkanCommandPool::create(device, &pool_info);
}

void VulkanCommandPool::destroy(
		VkDevice device, VulkanCommandPool& command_pool) {
	vkDestroyCommandPool(device, command_pool.command_pool, nullptr);
}

VulkanCommandBuffer VulkanCommandPool::allocate_buffer(VkDevice device) {
	VkCommandBufferAllocateInfo cmd_alloc_info =
			vkinit::command_buffer_allocate_info(command_pool, 1);

	VulkanCommandBuffer command_buffer{ VK_NULL_HANDLE };
	VK_CHECK(vkAllocateCommandBuffers(
			device, &cmd_alloc_info, &command_buffer.command_buffer));

	return command_buffer;
}

std::vector<VulkanCommandBuffer> VulkanCommandPool::allocate_buffer(
		VkDevice device, const uint32_t count) {
	VkCommandBufferAllocateInfo cmd_alloc_info =
			vkinit::command_buffer_allocate_info(command_pool, count);

	std::vector<VulkanCommandBuffer> command_buffers(count);

	// retrieve VkCommandBuffer* from our type
	std::vector<VkCommandBuffer*> command_buffer_ptrs(count);
	std::transform(command_buffers.begin(), command_buffers.end(),
			command_buffer_ptrs.begin(),
			[](VulkanCommandBuffer& cmd_buffer) -> VkCommandBuffer* {
				return &cmd_buffer.command_buffer;
			});

	VK_CHECK(vkAllocateCommandBuffers(
			device, &cmd_alloc_info, command_buffer_ptrs.front()));

	return command_buffers;
}

void VulkanCommandPool::reset(VkDevice device) {
	vkResetCommandPool(
			device, command_pool, VK_COMMAND_POOL_RESET_FLAG_BITS_MAX_ENUM);
}

void VulkanCommandBuffer::begin(VkBufferUsageFlags flags) {
	VkCommandBufferBeginInfo info = vkinit::command_buffer_begin_info(flags);
	vkBeginCommandBuffer(command_buffer, &info);
}

void VulkanCommandBuffer::end() { vkEndCommandBuffer(command_buffer); }

void VulkanCommandBuffer::reset(VkCommandBufferResetFlags flags) {
	vkResetCommandBuffer(command_buffer, flags);
}

void VulkanCommandBuffer::submit(VkQueue queue, VkFence fence,
		const VkSemaphoreSubmitInfo* wait_semaphore,
		const VkSemaphoreSubmitInfo* signal_semaphore) {
	VkCommandBufferSubmitInfo cmd_info =
			vkinit::command_buffer_submit_info(command_buffer);

	VkSubmitInfo2 submit =
			vkinit::submit_info(&cmd_info, wait_semaphore, signal_semaphore);

	VK_CHECK(vkQueueSubmit2(queue, 1, &submit, fence));
}

void VulkanCommandBuffer::begin_rendering(VkExtent2D draw_extent,
		uint32_t color_attachment_count,
		const VkRenderingAttachmentInfo* color_attachments,
		const VkRenderingAttachmentInfo* depth_attachment) {
	VkRenderingInfo render_info = {
		.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
		.pNext = nullptr,
		.renderArea = VkRect2D{ VkOffset2D{ 0, 0 }, draw_extent },
		.layerCount = 1,
		.colorAttachmentCount = color_attachment_count,
		.pColorAttachments = color_attachments,
		.pDepthAttachment = depth_attachment,
		.pStencilAttachment = nullptr,
	};

	vkCmdBeginRendering(command_buffer, &render_info);
}

void VulkanCommandBuffer::end_rendering() { vkCmdEndRendering(command_buffer); }

void VulkanCommandBuffer::bind_pipeline(const VulkanPipeline& pipeline) {
	vkCmdBindPipeline(
			command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);
}

void VulkanCommandBuffer::bind_pipeline(const VulkanComputePipeline& pipeline) {
	vkCmdBindPipeline(
			command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.pipeline);
}

void VulkanCommandBuffer::bind_index_buffer(const VulkanBuffer& index_buffer,
		VkDeviceSize offset, VkIndexType index_type) {
	vkCmdBindIndexBuffer(
			command_buffer, index_buffer.buffer, offset, index_type);
}

void VulkanCommandBuffer::draw_indexed(uint32_t index_count,
		uint32_t instance_count, uint32_t first_index, int32_t vertex_offset,
		uint32_t first_instance) {
	vkCmdDrawIndexed(command_buffer, index_count, instance_count, first_index,
			vertex_offset, first_instance);
}

void VulkanCommandBuffer::dispatch(uint32_t group_count_x,
		uint32_t group_count_y, uint32_t group_count_z) {
	vkCmdDispatch(command_buffer, group_count_x, group_count_y, group_count_z);
}

void VulkanCommandBuffer::bind_descriptor_sets(
		const VulkanPipelineLayout& pipeline_layout, uint32_t first_set,
		uint32_t descriptor_set_count, const VkDescriptorSet* descriptor_sets,
		VkPipelineBindPoint bind_point) {
	vkCmdBindDescriptorSets(command_buffer, bind_point, pipeline_layout.layout,
			first_set, descriptor_set_count, descriptor_sets, 0, nullptr);
}

void VulkanCommandBuffer::push_constants(
		const VulkanPipelineLayout& pipeline_layout,
		VkShaderStageFlags shader_stages, VkDeviceSize offset, uint32_t size,
		const void* push_constants) {
	vkCmdPushConstants(command_buffer, pipeline_layout.layout, shader_stages,
			offset, size, push_constants);
}

void VulkanCommandBuffer::set_viewport(Vec2f size) {
	VkViewport viewport = {
		.x = 0,
		.y = 0,
		.width = size.x,
		.height = size.y,
		.minDepth = 0.0f,
		.maxDepth = 1.0f,
	};

	vkCmdSetViewport(command_buffer, 0, 1, &viewport);
}

void VulkanCommandBuffer::set_scissor(VkExtent2D size, VkOffset2D offset) {
	VkRect2D scissor = {
		.offset = offset,
		.extent = size,
	};

	vkCmdSetScissor(command_buffer, 0, 1, &scissor);
}

void VulkanCommandBuffer::clear_color_image(VkImage image,
		VkImageLayout image_layout, const VkClearColorValue* clear_color,
		VkImageAspectFlags aspect_flags) {
	VkImageSubresourceRange image_range = {
		.aspectMask = aspect_flags,
		.levelCount = 1,
		.layerCount = 1,
	};
	vkCmdClearColorImage(
			command_buffer, image, image_layout, clear_color, 1, &image_range);
}

void VulkanCommandBuffer::copy_buffer(const VulkanBuffer& src_buffer,
		VulkanBuffer& dst_buffer, uint32_t region_count,
		const VkBufferCopy* regions) {
	vkCmdCopyBuffer(command_buffer, src_buffer.buffer, dst_buffer.buffer,
			region_count, regions);
}

void VulkanCommandBuffer::copy_buffer_to_image(const VulkanBuffer& src_buffer,
		Ref<VulkanImage> dst_image, VkImageLayout image_layout,
		uint32_t region_count, const VkBufferImageCopy* regions) {
	vkCmdCopyBufferToImage(command_buffer, src_buffer.buffer, dst_image->image,
			image_layout, region_count, regions);
}

void VulkanCommandBuffer::copy_image_to_image(
		Ref<VulkanImage> src_image, Ref<VulkanImage> dst_image) {
	copy_image_to_image(src_image->image, dst_image->image,
			{ src_image->image_extent.width, src_image->image_extent.height },
			{ dst_image->image_extent.width, dst_image->image_extent.height });
}

void VulkanCommandBuffer::copy_image_to_image(VkImage src_image,
		VkImage dst_image, const VkExtent2D& src_extent,
		const VkExtent2D& dst_extent) {
	VkImageBlit2 blit_region = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
		.pNext = nullptr,
	};

	blit_region.srcOffsets[1].x = src_extent.width;
	blit_region.srcOffsets[1].y = src_extent.height;
	blit_region.srcOffsets[1].z = 1;

	blit_region.dstOffsets[1].x = dst_extent.width;
	blit_region.dstOffsets[1].y = dst_extent.height;
	blit_region.dstOffsets[1].z = 1;

	blit_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blit_region.srcSubresource.baseArrayLayer = 0;
	blit_region.srcSubresource.layerCount = 1;
	blit_region.srcSubresource.mipLevel = 0;

	blit_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blit_region.dstSubresource.baseArrayLayer = 0;
	blit_region.dstSubresource.layerCount = 1;
	blit_region.dstSubresource.mipLevel = 0;

	VkBlitImageInfo2 blit_info{
		.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2,
		.pNext = nullptr,
		.srcImage = src_image,
		.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		.dstImage = dst_image,
		.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		.regionCount = 1,
		.pRegions = &blit_region,
		.filter = VK_FILTER_LINEAR,
	};

	vkCmdBlitImage2(command_buffer, &blit_info);
}

void VulkanCommandBuffer::transition_image(Ref<VulkanImage> image,
		VkImageLayout current_layout, VkImageLayout new_layout) {
	transition_image(image->image, current_layout, new_layout);
}

void VulkanCommandBuffer::transition_image(
		VkImage image, VkImageLayout current_layout, VkImageLayout new_layout) {
	VkImageAspectFlags aspect_mask =
			(new_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
			? VK_IMAGE_ASPECT_DEPTH_BIT
			: VK_IMAGE_ASPECT_COLOR_BIT;

	VkImageMemoryBarrier2 image_barrier = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
		.pNext = nullptr,
		.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
		.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
		.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
		.dstAccessMask =
				VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT,
		.oldLayout = current_layout,
		.newLayout = new_layout,
		.image = image,
		.subresourceRange = vkinit::image_subresource_range(aspect_mask),
	};

	VkDependencyInfo dep_info = {
		.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
		.pNext = nullptr,
		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers = &image_barrier,
	};

	vkCmdPipelineBarrier2(command_buffer, &dep_info);
}
