#include "glitch/platform/vulkan/vk_backend.h"

#include <vulkan/vulkan_core.h>

namespace gl {

void VulkanRenderBackend::command_immediate_submit(
		std::function<void(CommandBuffer p_cmd)>&& p_function,
		QueueType p_queue_type) {
	std::mutex& cmd_mutex = (p_queue_type == QueueType::TRANSFER)
			? imm_cmd_transfer_mutex
			: imm_cmd_graphics_mutex;

	std::scoped_lock lock(cmd_mutex);

	ImmediateBuffer* imm = (p_queue_type == QueueType::TRANSFER)
			? &imm_transfer
			: &imm_graphics;

	VK_CHECK(vkResetFences(device, 1, (VkFence*)&imm->fence));

	command_reset(imm->command_buffer);

	command_begin(imm->command_buffer);
	{
		// run the command
		p_function(imm->command_buffer);
	}
	command_end(imm->command_buffer);

	// submit command buffer to the queue and execute it.
	// imm_fence will now block until the graphic commands finish
	// execution
	queue_submit((p_queue_type == QueueType::TRANSFER)
					? queue_get(QueueType::TRANSFER)
					: queue_get(QueueType::GRAPHICS),
			imm->command_buffer, imm->fence);

	// wait till the operation finishes
	VK_CHECK(vkWaitForFences(
			device, 1, (VkFence*)&imm->fence, true, UINT64_MAX));
}

CommandPool VulkanRenderBackend::command_pool_create(CommandQueue p_queue) {
	VulkanQueue* queue = (VulkanQueue*)p_queue;

	VkCommandPoolCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	create_info.queueFamilyIndex = queue->queue_family;
	create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	VkCommandPool vk_command_pool = VK_NULL_HANDLE;
	VK_CHECK(vkCreateCommandPool(
			device, &create_info, nullptr, &vk_command_pool));

	return CommandPool(vk_command_pool);
}

void VulkanRenderBackend::command_pool_free(CommandPool p_command_pool) {
	VkCommandPool command_pool = (VkCommandPool)p_command_pool;

	vkDestroyCommandPool(device, command_pool, nullptr);
}

CommandBuffer VulkanRenderBackend::command_pool_allocate(
		CommandPool p_command_pool) {
	VkCommandPool command_pool = (VkCommandPool)p_command_pool;

	VkCommandBufferAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.pNext = nullptr;
	alloc_info.commandPool = command_pool;
	alloc_info.commandBufferCount = 1;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	VkCommandBuffer vk_command_buffer = VK_NULL_HANDLE;
	VK_CHECK(vkAllocateCommandBuffers(device, &alloc_info, &vk_command_buffer));

	return CommandBuffer(vk_command_buffer);
}

std::vector<CommandBuffer> VulkanRenderBackend::command_pool_allocate(
		CommandPool p_command_pool, const uint32_t p_count) {
	VkCommandPool command_pool = (VkCommandPool)p_command_pool;

	VkCommandBufferAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.pNext = nullptr;
	alloc_info.commandPool = command_pool;
	alloc_info.commandBufferCount = p_count;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	std::vector<CommandBuffer> command_buffers(p_count);
	VK_CHECK(vkAllocateCommandBuffers(
			device, &alloc_info, (VkCommandBuffer*)&command_buffers.front()));

	return command_buffers;
}

void VulkanRenderBackend::command_pool_reset(CommandPool p_command_pool) {
	vkResetCommandPool(device, (VkCommandPool)p_command_pool,
			VK_COMMAND_POOL_RESET_FLAG_BITS_MAX_ENUM);
}

void VulkanRenderBackend::command_begin(CommandBuffer p_cmd) {
	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.pNext = nullptr;
	begin_info.pInheritanceInfo = nullptr;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer((VkCommandBuffer)p_cmd, &begin_info);
}

void VulkanRenderBackend::command_end(CommandBuffer p_cmd) {
	vkEndCommandBuffer((VkCommandBuffer)p_cmd);
}

void VulkanRenderBackend::command_reset(CommandBuffer p_cmd) {
	vkResetCommandBuffer((VkCommandBuffer)p_cmd, 0);
}

void VulkanRenderBackend::command_begin_rendering(CommandBuffer p_cmd,
		const glm::uvec2& p_draw_extent,
		VectorView<RenderingAttachment> p_color_attachments,
		Image p_depth_attachment) {
	std::vector<VkRenderingAttachmentInfo> color_attachment_infos;
	for (const auto& attachment : p_color_attachments) {
		VulkanImage* vk_image = (VulkanImage*)attachment.image;

		VkRenderingAttachmentInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		info.pNext = nullptr;
		info.imageView = vk_image->vk_image_view;
		info.imageLayout = static_cast<VkImageLayout>(attachment.layout);
		info.loadOp = static_cast<VkAttachmentLoadOp>(attachment.load_op);
		info.storeOp = static_cast<VkAttachmentStoreOp>(attachment.store_op);
		// Set clear color if any provided
		if (attachment.clear_color != COLOR_TRANSPARENT) {
			info.clearValue = { {
					attachment.clear_color.r,
					attachment.clear_color.g,
					attachment.clear_color.b,
					attachment.clear_color.a,
			} };
		}

		// Resolve image (MSAA) if provided
		if (attachment.resolve_image != GL_NULL_HANDLE) {
			VulkanImage* vk_resolve = (VulkanImage*)attachment.resolve_image;

			info.resolveMode =
					static_cast<VkResolveModeFlagBits>(attachment.resolve_mode);
			info.resolveImageView = vk_resolve->vk_image_view;
			info.resolveImageLayout =
					static_cast<VkImageLayout>(attachment.resolve_layout);
		}

		color_attachment_infos.push_back(info);
	}

	VkRenderingAttachmentInfo depth_attachment_info = {};
	if (p_depth_attachment) {
		VulkanImage* vk_depth_image = (VulkanImage*)p_depth_attachment;

		depth_attachment_info.sType =
				VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		depth_attachment_info.pNext = nullptr;
		depth_attachment_info.imageView = vk_depth_image->vk_image_view;
		depth_attachment_info.imageLayout =
				VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
		depth_attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depth_attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depth_attachment_info.clearValue.depthStencil.depth = 1.0f;
	}

	VkRenderingInfo render_info = {};
	render_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	render_info.renderArea = VkRect2D{ VkOffset2D{ 0, 0 },
		{ p_draw_extent.x, p_draw_extent.y } };
	render_info.layerCount = 1;
	render_info.colorAttachmentCount = color_attachment_infos.size();
	render_info.pColorAttachments = color_attachment_infos.data();
	render_info.pDepthAttachment =
			p_depth_attachment == nullptr ? nullptr : &depth_attachment_info;
	render_info.pStencilAttachment = nullptr;

	vkCmdBeginRendering((VkCommandBuffer)p_cmd, &render_info);
}

void VulkanRenderBackend::command_end_rendering(CommandBuffer p_cmd) {
	vkCmdEndRendering((VkCommandBuffer)p_cmd);
}

void VulkanRenderBackend::command_begin_render_pass(CommandBuffer p_cmd,
		RenderPass p_render_pass, FrameBuffer framebuffer,
		const glm::uvec2& p_draw_extent, Color p_clear_color) {
	VulkanRenderPass* render_pass_info = (VulkanRenderPass*)p_render_pass;

	std::vector<VkClearValue> clear_values(
			render_pass_info->attachments.size());
	for (size_t i = 0; i < render_pass_info->attachments.size(); ++i) {
		VkClearValue& clear = clear_values[i];
		const RenderPassAttachment& attachment =
				render_pass_info->attachments[i];

		if (attachment.is_depth_attachment) {
			clear.depthStencil = { 1.0f, 0 };
		} else {
			clear.color = { { p_clear_color.r, p_clear_color.g, p_clear_color.b,
					1.0f } };
		}
	}

	VkRenderPassBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	begin_info.renderPass = render_pass_info->vk_render_pass;
	begin_info.framebuffer = VkFramebuffer(framebuffer);
	begin_info.renderArea.offset = { 0, 0 };
	begin_info.renderArea.extent = { p_draw_extent.x, p_draw_extent.y };
	begin_info.clearValueCount = clear_values.size();
	begin_info.pClearValues = clear_values.data();

	vkCmdBeginRenderPass(
			(VkCommandBuffer)p_cmd, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanRenderBackend::command_end_render_pass(CommandBuffer p_cmd) {
	vkCmdEndRenderPass((VkCommandBuffer)p_cmd);
}

void VulkanRenderBackend::command_clear_color(CommandBuffer p_cmd,
		Image p_image, const Color& p_clear_color,
		ImageAspectFlags p_image_aspect) {
	VulkanImage* image = (VulkanImage*)p_image;

	VkClearColorValue clear_color = {};
	static_assert(sizeof(VkClearColorValue) == sizeof(Color));
	memcpy(&clear_color.float32, &p_clear_color.r, sizeof(Color));

	VkImageSubresourceRange image_range = {};
	image_range.aspectMask = [p_image_aspect]() -> VkImageAspectFlags {
		switch (p_image_aspect) {
			case IMAGE_ASPECT_COLOR_BIT:
				return VK_IMAGE_ASPECT_COLOR_BIT;
			case IMAGE_ASPECT_DEPTH_BIT:
				return VK_IMAGE_ASPECT_DEPTH_BIT;
			case IMAGE_ASPECT_STENCIL_BIT:
				return VK_IMAGE_ASPECT_STENCIL_BIT;
			default:
				return VK_IMAGE_ASPECT_NONE;
		}
	}();
	image_range.levelCount = 1;
	image_range.layerCount = 1;

	vkCmdClearColorImage((VkCommandBuffer)p_cmd, image->vk_image,
			VK_IMAGE_LAYOUT_GENERAL, &clear_color, 1, &image_range);
}

void VulkanRenderBackend::command_bind_graphics_pipeline(
		CommandBuffer p_cmd, Pipeline p_pipeline) {
	VulkanPipeline* pipeline = (VulkanPipeline*)p_pipeline;

	vkCmdBindPipeline((VkCommandBuffer)p_cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipeline->vk_pipeline);
}

void VulkanRenderBackend::command_bind_compute_pipeline(
		CommandBuffer p_cmd, Pipeline p_pipeline) {
	VulkanPipeline* pipeline = (VulkanPipeline*)p_pipeline;

	vkCmdBindPipeline((VkCommandBuffer)p_cmd, VK_PIPELINE_BIND_POINT_COMPUTE,
			pipeline->vk_pipeline);
}

void VulkanRenderBackend::command_bind_vertex_buffers(CommandBuffer p_cmd,
		uint32_t p_first_binding, std::vector<Buffer> p_vertex_buffers,
		std::vector<uint64_t> p_offsets) {
	GL_ASSERT(p_vertex_buffers.size() == p_offsets.size(),
			"Buffer array size and offset array size does not match");

	std::vector<VkBuffer> vk_buffers(p_vertex_buffers.size());
	for (size_t i = 0; i < p_vertex_buffers.size(); ++i) {
		VulkanBuffer* vk_buffer = (VulkanBuffer*)p_vertex_buffers[i];
		vk_buffers[i] = vk_buffer->vk_buffer;
	}

	vkCmdBindVertexBuffers((VkCommandBuffer)p_cmd, p_first_binding,
			static_cast<uint32_t>(vk_buffers.size()), vk_buffers.data(),
			p_offsets.data());
}

void VulkanRenderBackend::command_bind_index_buffer(CommandBuffer p_cmd,
		Buffer p_index_buffer, uint64_t p_offset, IndexType p_index_type) {
	VulkanBuffer* index_buffer = (VulkanBuffer*)p_index_buffer;

	vkCmdBindIndexBuffer((VkCommandBuffer)p_cmd, index_buffer->vk_buffer,
			p_offset,
			p_index_type == IndexType::UINT16 ? VK_INDEX_TYPE_UINT16
											  : VK_INDEX_TYPE_UINT32);
}

void VulkanRenderBackend::command_draw(CommandBuffer p_cmd,
		uint32_t p_vertex_count, uint32_t p_instance_count,
		uint32_t p_first_vertex, uint32_t p_first_instance) {
	vkCmdDraw((VkCommandBuffer)p_cmd, p_vertex_count, p_instance_count,
			p_first_vertex, p_first_instance);
}

void VulkanRenderBackend::command_draw_indexed(CommandBuffer p_cmd,
		uint32_t p_index_count, uint32_t p_instance_count,
		uint32_t p_first_index, int32_t p_vertex_offset,
		uint32_t p_first_instance) {
	vkCmdDrawIndexed((VkCommandBuffer)p_cmd, p_index_count, p_instance_count,
			p_first_index, p_vertex_offset, p_first_instance);
}

void VulkanRenderBackend::command_draw_indexed_indirect(CommandBuffer p_cmd,
		Buffer p_buffer, uint64_t p_offset, uint32_t p_draw_count,
		uint32_t p_stride) {
	VulkanBuffer* buffer = (VulkanBuffer*)p_buffer;

	vkCmdDrawIndexedIndirect((VkCommandBuffer)p_cmd, buffer->vk_buffer,
			p_offset, p_draw_count, p_stride);
}

void VulkanRenderBackend::command_dispatch(CommandBuffer p_cmd,
		uint32_t p_group_count_x, uint32_t p_group_count_y,
		uint32_t p_group_count_z) {
	vkCmdDispatch((VkCommandBuffer)p_cmd, p_group_count_x, p_group_count_y,
			p_group_count_z);
}

void VulkanRenderBackend::command_bind_uniform_sets(CommandBuffer p_cmd,
		Shader p_shader, uint32_t p_first_set,
		VectorView<UniformSet> p_uniform_sets, PipelineType p_type) {
	VulkanShader* shader = (VulkanShader*)p_shader;

	std::vector<VkDescriptorSet> uniform_sets;
	for (uint32_t i = 0; i < p_uniform_sets.size(); i++) {
		VulkanUniformSet* uniform_set = (VulkanUniformSet*)p_uniform_sets[i];

		uniform_sets.push_back(uniform_set->vk_descriptor_set);
	}

	vkCmdBindDescriptorSets((VkCommandBuffer)p_cmd,
			p_type == PipelineType::GRAPHICS ? VK_PIPELINE_BIND_POINT_GRAPHICS
											 : VK_PIPELINE_BIND_POINT_COMPUTE,
			shader->pipeline_layout, p_first_set, p_uniform_sets.size(),
			uniform_sets.data(), 0, nullptr);
}

void VulkanRenderBackend::command_push_constants(CommandBuffer p_cmd,
		Shader p_shader, uint64_t p_offset, uint32_t p_size,
		const void* p_push_constants) {
	VulkanShader* shader = (VulkanShader*)p_shader;

	vkCmdPushConstants((VkCommandBuffer)p_cmd, shader->pipeline_layout,
			shader->push_constant_stages, p_offset, p_size, p_push_constants);
}

void VulkanRenderBackend::command_set_viewport(
		CommandBuffer p_cmd, const glm::uvec2& size) {
	VkViewport viewport = {
		.x = 0,
		.y = 0,
		.width = (float)size.x,
		.height = (float)size.y,
		.minDepth = 0.0f,
		.maxDepth = 1.0f,
	};

	vkCmdSetViewport((VkCommandBuffer)p_cmd, 0, 1, &viewport);
}

void VulkanRenderBackend::command_set_scissor(CommandBuffer p_cmd,
		const glm::uvec2& p_size, const glm::uvec2& p_offset) {
	VkRect2D scissor = {};
	memcpy(&scissor.extent, &p_size, sizeof(VkExtent2D));
	memcpy(&scissor.offset, &p_offset, sizeof(VkExtent2D));

	vkCmdSetScissor((VkCommandBuffer)p_cmd, 0, 1, &scissor);
}

void VulkanRenderBackend::command_set_depth_bias(CommandBuffer p_cmd,
		float p_depth_bias_constant_factor, float p_depth_bias_clamp,
		float p_depth_bias_slope_factor) {
	vkCmdSetDepthBias((VkCommandBuffer)p_cmd, p_depth_bias_constant_factor,
			p_depth_bias_clamp, p_depth_bias_slope_factor);
}

void VulkanRenderBackend::command_copy_buffer(CommandBuffer p_cmd,
		Buffer p_src_buffer, Buffer p_dst_buffer,
		VectorView<BufferCopyRegion> p_regions) {
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

void VulkanRenderBackend::command_copy_buffer_to_image(CommandBuffer p_cmd,
		Buffer p_src_buffer, Image p_dst_image,
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

void VulkanRenderBackend::command_copy_image_to_image(CommandBuffer p_cmd,
		Image p_src_image, Image p_dst_image, const glm::uvec2& p_src_extent,
		const glm::uvec2& p_dst_extent, uint32_t p_src_mip_level,
		uint32_t p_dst_mip_level) {
	VkImageBlit2 blit_region = {};
	blit_region.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2;

	blit_region.srcOffsets[1].x = p_src_extent.x;
	blit_region.srcOffsets[1].y = p_src_extent.y;
	blit_region.srcOffsets[1].z = 1;

	blit_region.dstOffsets[1].x = p_dst_extent.x;
	blit_region.dstOffsets[1].y = p_dst_extent.y;
	blit_region.dstOffsets[1].z = 1;

	blit_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blit_region.srcSubresource.baseArrayLayer = 0;
	blit_region.srcSubresource.layerCount = 1;
	blit_region.srcSubresource.mipLevel = p_src_mip_level;

	blit_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blit_region.dstSubresource.baseArrayLayer = 0;
	blit_region.dstSubresource.layerCount = 1;
	blit_region.dstSubresource.mipLevel = p_dst_mip_level;

	VulkanImage* src_image = (VulkanImage*)p_src_image;
	VulkanImage* dst_image = (VulkanImage*)p_dst_image;

	VkBlitImageInfo2 blit_info = {};
	blit_info.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2;
	blit_info.srcImage = src_image->vk_image;
	blit_info.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	blit_info.dstImage = dst_image->vk_image;
	blit_info.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	blit_info.regionCount = 1;
	blit_info.pRegions = &blit_region;
	blit_info.filter = VK_FILTER_LINEAR;

	vkCmdBlitImage2((VkCommandBuffer)p_cmd, &blit_info);
}

void VulkanRenderBackend::command_transition_image(CommandBuffer p_cmd,
		Image p_image, ImageLayout p_current_layout, ImageLayout p_new_layout,
		uint32_t p_base_mip_level, uint32_t p_level_count) {
	VkImageAspectFlags aspect_mask =
			(p_current_layout == ImageLayout::DEPTH_ATTACHMENT_OPTIMAL ||
					p_new_layout == ImageLayout::DEPTH_ATTACHMENT_OPTIMAL)
			? VK_IMAGE_ASPECT_DEPTH_BIT
			: VK_IMAGE_ASPECT_COLOR_BIT;

	VkImageLayout vk_current_layout =
			static_cast<VkImageLayout>(p_current_layout);
	VkImageLayout vk_new_layout = static_cast<VkImageLayout>(p_new_layout);

	VulkanImage* image = (VulkanImage*)p_image;

	VkImageSubresourceRange sub_image = {};
	sub_image.aspectMask = aspect_mask;
	sub_image.baseMipLevel = p_base_mip_level;
	sub_image.levelCount = p_level_count;
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

} //namespace gl