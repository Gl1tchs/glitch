#include "backends/vulkan/vk_commands.h"

#include "backends/vulkan/vk_buffer.h"
#include "backends/vulkan/vk_common.h"
#include "backends/vulkan/vk_context.h"
#include "backends/vulkan/vk_image.h"
#include "backends/vulkan/vk_shader.h"
#include "renderer/render_backend.h"
#include "renderer/types.h"

#include <vulkan/vulkan_core.h>

namespace vk {

CommandPool command_pool_create(
		Context p_context, CommandBuffer p_cmd, CommandQueue p_queue) {
	VulkanContext* context = (VulkanContext*)p_context;
	VulkanQueue* queue = (VulkanQueue*)p_queue;

	VkCommandPoolCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	create_info.queueFamilyIndex = queue->queue_family;

	VkCommandPool vk_command_pool = VK_NULL_HANDLE;
	VK_CHECK(vkCreateCommandPool(
			context->device, &create_info, nullptr, &vk_command_pool));

	return CommandPool(vk_command_pool);
}

void command_pool_free(
		Context p_context, CommandBuffer p_cmd, CommandPool p_command_pool) {
	VulkanContext* context = (VulkanContext*)p_context;

	VkCommandPool command_pool = (VkCommandPool)p_command_pool;

	vkDestroyCommandPool(context->device, command_pool, nullptr);
}

CommandBuffer command_pool_allocate(
		Context p_context, CommandPool p_command_pool) {
	VulkanContext* context = (VulkanContext*)p_context;

	VkCommandPool command_pool = (VkCommandPool)p_command_pool;

	VkCommandBufferAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.pNext = nullptr;
	alloc_info.commandPool = command_pool;
	alloc_info.commandBufferCount = 1;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	VkCommandBuffer vk_command_buffer = VK_NULL_HANDLE;
	VK_CHECK(vkAllocateCommandBuffers(
			context->device, &alloc_info, &vk_command_buffer));

	return CommandBuffer(vk_command_buffer);
}

std::vector<CommandBuffer> command_pool_allocate(
		Context p_context, CommandPool p_command_pool, const uint32_t p_count) {
	VulkanContext* context = (VulkanContext*)p_context;

	VkCommandPool command_pool = (VkCommandPool)p_command_pool;

	VkCommandBufferAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.pNext = nullptr;
	alloc_info.commandPool = command_pool;
	alloc_info.commandBufferCount = p_count;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	std::vector<CommandBuffer> command_buffers(p_count);
	VK_CHECK(vkAllocateCommandBuffers(context->device, &alloc_info,
			(VkCommandBuffer*)&command_buffers.front()));

	return command_buffers;
}

void command_pool_reset(Context p_context, CommandPool p_command_pool) {
	VulkanContext* context = (VulkanContext*)p_context;

	vkResetCommandPool(context->device, (VkCommandPool)p_command_pool,
			VK_COMMAND_POOL_RESET_FLAG_BITS_MAX_ENUM);
}

void command_begin(CommandBuffer p_cmd) {
	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.pNext = nullptr;
	begin_info.pInheritanceInfo = nullptr;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer((VkCommandBuffer)p_cmd, &begin_info);
}

void command_end(CommandBuffer p_cmd) {
	vkEndCommandBuffer((VkCommandBuffer)p_cmd);
}

void command_reset(CommandBuffer p_cmd) {
	vkResetCommandBuffer((VkCommandBuffer)p_cmd, 0);
}

void command_submit(CommandBuffer p_cmd, CommandQueue p_queue, Fence p_fence,
		Semaphore p_wait_semaphore, Semaphore p_signal_semaphore) {
	VkCommandBufferSubmitInfo cmd_info{};
	cmd_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
	cmd_info.pNext = nullptr;
	cmd_info.commandBuffer = (VkCommandBuffer)p_cmd;
	cmd_info.deviceMask = 0;

	VkSemaphoreSubmitInfo wait_semaphore_info{};
	wait_semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
	wait_semaphore_info.pNext = nullptr;
	wait_semaphore_info.semaphore = (VkSemaphore)p_wait_semaphore;
	wait_semaphore_info.stageMask =
			VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT; // TODO get as
															 // parameter
	wait_semaphore_info.deviceIndex = 0;
	wait_semaphore_info.value = 1;

	VkSemaphoreSubmitInfo signal_semaphore_info{};
	wait_semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
	wait_semaphore_info.pNext = nullptr;
	wait_semaphore_info.semaphore = (VkSemaphore)p_signal_semaphore;
	wait_semaphore_info.stageMask =
			VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT; // TODO get as parameter
	wait_semaphore_info.deviceIndex = 0;
	wait_semaphore_info.value = 1;

	VkSubmitInfo2 submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
	submit_info.pNext = nullptr;
	submit_info.waitSemaphoreInfoCount = p_wait_semaphore == nullptr ? 0 : 1;
	submit_info.pWaitSemaphoreInfos = &signal_semaphore_info;
	submit_info.signalSemaphoreInfoCount =
			p_signal_semaphore == nullptr ? 0 : 1;
	submit_info.pSignalSemaphoreInfos = &signal_semaphore_info;
	submit_info.commandBufferInfoCount = 1;
	submit_info.pCommandBufferInfos = &cmd_info;

	VulkanQueue* queue = (VulkanQueue*)p_queue;

	VK_CHECK(vkQueueSubmit2(queue->queue, 1, &submit_info, (VkFence)p_fence));
}

void command_begin_rendering(CommandBuffer p_cmd, const Vec2u& p_draw_extent,
		VectorView<Image> p_color_attachments, Image p_depth_attachment) {
	std::vector<VkRenderingAttachmentInfo> color_attachment_infos(
			p_color_attachments.size());
	for (uint32_t i = 0; i < p_color_attachments.size(); i++) {
		VulkanImage* vk_image = (VulkanImage*)p_color_attachments[i];

		auto& info = color_attachment_infos[i];
		info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		info.pNext = nullptr;
		info.imageView = vk_image->vk_image_view;
		info.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		info.loadOp =
				VK_ATTACHMENT_LOAD_OP_LOAD; // TODO VK_ATTACHMENT_LOAD_OP_CLEAR
		info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		// info.clearValue = clear_value; // TODO
	}

	VulkanImage* vk_depth_image = (VulkanImage*)p_depth_attachment;

	VkRenderingAttachmentInfo depth_attachment_info{};
	depth_attachment_info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	depth_attachment_info.pNext = nullptr;
	depth_attachment_info.imageView = vk_depth_image->vk_image_view;
	depth_attachment_info.imageLayout =
			VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
	depth_attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depth_attachment_info.clearValue.depthStencil.depth = 1.0f;

	VkRenderingInfo render_info = {};
	render_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	render_info.pNext = nullptr;
	render_info.renderArea = VkRect2D{ VkOffset2D{ 0, 0 },
		{ p_draw_extent.x, p_draw_extent.y } };
	render_info.layerCount = 1;
	render_info.colorAttachmentCount = p_color_attachments.size();
	render_info.pColorAttachments = color_attachment_infos.data();
	render_info.pDepthAttachment = &depth_attachment_info;
	render_info.pStencilAttachment = nullptr;

	vkCmdBeginRendering((VkCommandBuffer)p_cmd, &render_info);
}

void command_end_rendering(CommandBuffer p_cmd) {
	vkCmdEndRendering((VkCommandBuffer)p_cmd);
}

void command_bind_graphics_pipeline(CommandBuffer p_cmd, Pipeline p_pipeline) {
	vkCmdBindPipeline((VkCommandBuffer)p_cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
			(VkPipeline)p_pipeline);
}

void command_bind_compute_pipeline(CommandBuffer p_cmd, Pipeline p_pipeline) {
	vkCmdBindPipeline((VkCommandBuffer)p_cmd, VK_PIPELINE_BIND_POINT_COMPUTE,
			(VkPipeline)p_pipeline);
}

void command_bind_index_buffer(CommandBuffer p_cmd, Buffer p_index_buffer,
		uint64_t p_offset, IndexType p_index_type) {
	VulkanBuffer* index_buffer = (VulkanBuffer*)p_index_buffer;

	vkCmdBindIndexBuffer((VkCommandBuffer)p_cmd, index_buffer->vk_buffer,
			p_offset,
			p_index_type == INDEX_TYPE_UINT16 ? VK_INDEX_TYPE_UINT16
											  : VK_INDEX_TYPE_UINT32);
}

void command_draw_indexed(CommandBuffer p_cmd, uint32_t p_index_count,
		uint32_t p_instance_count, uint32_t p_first_index,
		int32_t p_vertex_offset, uint32_t p_first_instance) {
	vkCmdDrawIndexed((VkCommandBuffer)p_cmd, p_index_count, p_instance_count,
			p_first_index, p_vertex_offset, p_first_instance);
}

void command_draw_indexed_indirect(CommandBuffer p_cmd, Buffer p_buffer,
		uint64_t p_offset, uint32_t p_draw_count, uint32_t p_stride) {
	VulkanBuffer* buffer = (VulkanBuffer*)p_buffer;

	vkCmdDrawIndexedIndirect((VkCommandBuffer)p_cmd, buffer->vk_buffer,
			p_offset, p_draw_count, p_stride);
}

void command_dispatch(CommandBuffer p_cmd, uint32_t p_group_count_x,
		uint32_t p_group_count_y, uint32_t p_group_count_z) {
	vkCmdDispatch((VkCommandBuffer)p_cmd, p_group_count_x, p_group_count_y,
			p_group_count_z);
}

void command_bind_uniform_sets(CommandBuffer p_cmd, Shader p_shader,
		uint32_t p_first_set, VectorView<UniformSet> p_uniform_sets,
		PipelineType p_type) {
	VulkanShader* shader = (VulkanShader*)p_shader;

	vkCmdBindDescriptorSets((VkCommandBuffer)p_cmd,
			p_type == PIPELINE_TYPE_GRAPHICS ? VK_PIPELINE_BIND_POINT_GRAPHICS
											 : VK_PIPELINE_BIND_POINT_COMPUTE,
			shader->pipeline_layout, p_first_set, p_uniform_sets.size(),
			(const VkDescriptorSet*)p_uniform_sets.data(), 0, nullptr);
}

void command_push_constants(CommandBuffer p_cmd, Shader p_shader,
		uint64_t p_offset, uint32_t p_size, const void* p_push_constants) {
	VulkanShader* shader = (VulkanShader*)p_shader;

	vkCmdPushConstants((VkCommandBuffer)p_cmd, shader->pipeline_layout,
			VK_SHADER_STAGE_ALL, p_offset, p_size, p_push_constants);
}

void command_set_viewport(CommandBuffer p_cmd, Vec2f size) {
	VkViewport viewport = {
		.x = 0,
		.y = 0,
		.width = size.x,
		.height = size.y,
		.minDepth = 0.0f,
		.maxDepth = 1.0f,
	};

	vkCmdSetViewport((VkCommandBuffer)p_cmd, 0, 1, &viewport);
}

void command_set_scissor(
		CommandBuffer p_cmd, const Vec2u& p_size, const Vec2u& p_offset) {
	VkRect2D scissor = {};
	memcpy(&scissor.extent, &p_size, sizeof(VkExtent2D));
	memcpy(&scissor.offset, &p_offset, sizeof(VkExtent2D));

	vkCmdSetScissor((VkCommandBuffer)p_cmd, 0, 1, &scissor);
}

void command_copy_buffer(CommandBuffer p_cmd, Buffer p_src_buffer,
		Buffer p_dst_buffer, VectorView<BufferCopyRegion> p_regions) {
	VulkanBuffer* src_buffer = (VulkanBuffer*)p_src_buffer;
	VulkanBuffer* dst_buffer = (VulkanBuffer*)p_dst_buffer;

	static_assert(sizeof(BufferCopyRegion) == sizeof(VkBufferCopy));

	std::vector<VkBufferCopy> regions(p_regions.size());
	for (uint32_t i = 0; i < p_regions.size(); i++) {
		VkBufferCopy& copy = regions[i];
		memcpy(&copy, &p_regions[i], sizeof(VkBufferCopy));
	}

	vkCmdCopyBuffer((VkCommandBuffer)p_cmd, src_buffer->vk_buffer,
			dst_buffer->vk_buffer, p_regions.size(), regions.data());
}

void command_copy_buffer_to_image(CommandBuffer p_cmd, Buffer p_src_buffer,
		Image p_dst_image, uint32_t p_region_count,
		VectorView<BufferImageCopyRegion> p_regions) {
	VulkanBuffer* src_buffer = (VulkanBuffer*)p_src_buffer;
	VulkanImage* dst_image = (VulkanImage*)p_dst_image;

	static_assert(sizeof(BufferImageCopyRegion) == sizeof(VkBufferImageCopy));

	std::vector<VkBufferImageCopy> regions(p_regions.size());
	for (uint32_t i = 0; i < p_regions.size(); i++) {
		VkBufferImageCopy& copy = regions[i];
		memcpy(&copy, &p_regions[i], sizeof(VkBufferImageCopy));
	}

	vkCmdCopyBufferToImage((VkCommandBuffer)p_cmd, src_buffer->vk_buffer,
			dst_image->vk_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			p_regions.size(), regions.data());
}

void command_copy_image_to_image(CommandBuffer p_cmd, Image p_src_image,
		Image p_dst_image, const Vec2u& p_src_extent,
		const Vec2u& p_dst_extent) {
	VkImageBlit2 blit_region = {};
	blit_region.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2;
	blit_region.pNext = nullptr;

	blit_region.srcOffsets[1].x = p_src_extent.x;
	blit_region.srcOffsets[1].y = p_src_extent.y;
	blit_region.srcOffsets[1].z = 1;

	blit_region.dstOffsets[1].x = p_dst_extent.x;
	blit_region.dstOffsets[1].y = p_dst_extent.y;
	blit_region.dstOffsets[1].z = 1;

	blit_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blit_region.srcSubresource.baseArrayLayer = 0;
	blit_region.srcSubresource.layerCount = 1;
	blit_region.srcSubresource.mipLevel = 0;

	blit_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blit_region.dstSubresource.baseArrayLayer = 0;
	blit_region.dstSubresource.layerCount = 1;
	blit_region.dstSubresource.mipLevel = 0;

	VulkanImage* src_image = (VulkanImage*)p_src_image;
	VulkanImage* dst_image = (VulkanImage*)p_dst_image;

	VkBlitImageInfo2 blit_info = {};
	blit_info.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2;
	blit_info.pNext = nullptr;
	blit_info.srcImage = src_image->vk_image;
	blit_info.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	blit_info.dstImage = dst_image->vk_image;
	blit_info.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	blit_info.regionCount = 1;
	blit_info.pRegions = &blit_region;
	blit_info.filter = VK_FILTER_LINEAR;

	vkCmdBlitImage2((VkCommandBuffer)p_cmd, &blit_info);
}

void command_transition_image(CommandBuffer p_cmd, Image p_image,
		ImageLayout p_current_layout, ImageLayout p_new_layout) {
	VkImageAspectFlags aspect_mask =
			(p_new_layout == IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
			? VK_IMAGE_ASPECT_DEPTH_BIT
			: VK_IMAGE_ASPECT_COLOR_BIT;

	VkImageLayout vk_current_layout =
			static_cast<VkImageLayout>(p_current_layout);
	VkImageLayout vk_new_layout = static_cast<VkImageLayout>(p_new_layout);

	VulkanImage* image = (VulkanImage*)p_image;

	VkImageSubresourceRange sub_image = {};
	sub_image.aspectMask = aspect_mask;
	sub_image.baseMipLevel = 0;
	sub_image.levelCount = VK_REMAINING_MIP_LEVELS;
	sub_image.baseArrayLayer = 0;
	sub_image.layerCount = VK_REMAINING_ARRAY_LAYERS;

	VkImageMemoryBarrier2 image_barrier = {};
	image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
	image_barrier.pNext = nullptr;
	image_barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
	image_barrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
	image_barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
	image_barrier.dstAccessMask =
			VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
	image_barrier.oldLayout = vk_current_layout;
	image_barrier.newLayout = vk_new_layout;
	image_barrier.image = image->vk_image;
	image_barrier.subresourceRange = sub_image;

	VkDependencyInfo dep_info = {};
	dep_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	dep_info.pNext = nullptr;
	dep_info.imageMemoryBarrierCount = 1;
	dep_info.pImageMemoryBarriers = &image_barrier;

	vkCmdPipelineBarrier2((VkCommandBuffer)p_cmd, &dep_info);
}

} //namespace vk