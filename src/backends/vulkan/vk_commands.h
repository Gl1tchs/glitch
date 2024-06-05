#pragma once

#include "core/color.h"
#include "core/templates/vector_view.h"

#include "renderer/render_backend.h"
#include "renderer/types.h"

namespace vk {

CommandPool command_pool_create(
		Context p_context, CommandBuffer p_cmd, CommandQueue p_queue);

void command_pool_free(
		Context p_context, CommandBuffer p_cmd, CommandPool p_command_pool);

CommandBuffer command_pool_allocate(
		Context p_context, CommandPool p_command_pool);

std::vector<CommandBuffer> command_pool_allocate(
		Context p_context, CommandPool p_command_pool, const uint32_t count);

void command_pool_reset(Context p_context, CommandPool p_command_pool);

void command_begin(CommandBuffer p_cmd);

void command_end(CommandBuffer p_cmd);

void command_reset(CommandBuffer p_cmd);

void command_submit(CommandBuffer p_cmd, CommandQueue p_queue,
		Fence p_fence = nullptr, Semaphore p_wait_semaphore = nullptr,
		Semaphore p_signal_semaphore = nullptr);

void command_begin_rendering(CommandBuffer p_cmd, const Vec2u& p_draw_extent,
		VectorView<Image> p_color_attachments, Image p_depth_attachment);

void command_end_rendering(CommandBuffer p_cmd);

void command_bind_graphics_pipeline(CommandBuffer p_cmd, Pipeline p_pipeline);

void command_bind_compute_pipeline(CommandBuffer p_cmd, Pipeline p_pipeline);

void command_bind_index_buffer(CommandBuffer p_cmd, Buffer p_index_buffer,
		uint64_t p_offset, IndexType p_index_type);

void command_draw_indexed(CommandBuffer p_cmd, uint32_t p_index_count,
		uint32_t p_instance_count = 1, uint32_t p_first_index = 0,
		int32_t p_vertex_offset = 0, uint32_t p_first_instance = 0);

void command_draw_indexed_indirect(CommandBuffer p_cmd, Buffer p_buffer,
		uint64_t p_offset, uint32_t p_draw_count, uint32_t p_stride);

void command_dispatch(CommandBuffer p_cmd, uint32_t p_group_count_x,
		uint32_t p_group_count_y, uint32_t p_group_count_z);

void command_bind_uniform_sets(CommandBuffer p_cmd, Shader p_shader,
		uint32_t p_first_set, VectorView<UniformSet> p_uniform_sets,
		PipelineType p_type = PIPELINE_TYPE_GRAPHICS);

void command_push_constants(CommandBuffer p_cmd, Shader p_shader,
		uint64_t p_offset, uint32_t p_size, const void* p_push_constants);

void command_set_viewport(CommandBuffer p_cmd, Vec2f size);

void command_set_scissor(CommandBuffer p_cmd, const Vec2u& p_size,
		const Vec2u& p_offset = { 0, 0 });

void command_copy_buffer(CommandBuffer p_cmd, Buffer p_src_buffer,
		Buffer p_dst_buffer, VectorView<BufferCopyRegion> p_regions);

// image layout must be VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
void command_copy_buffer_to_image(CommandBuffer p_cmd, Buffer p_src_buffer,
		Image p_dst_image, VectorView<BufferImageCopyRegion> p_regions);

void command_copy_image_to_image(CommandBuffer p_cmd, Image p_src_image,
		Image p_dst_image, const Vec2u& p_src_extent,
		const Vec2u& p_dst_extent);

void command_transition_image(CommandBuffer p_cmd, Image p_image,
		ImageLayout p_current_layout, ImageLayout p_new_layout);

} //namespace vk
