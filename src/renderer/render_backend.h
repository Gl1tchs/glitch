#pragma once

#include "core/color.h"
#include "core/templates/bit_field.h"
#include "core/templates/vector_view.h"
#include "core/window.h"

#include "renderer/types.h"

class GL_API RenderBackend {
public:
	virtual ~RenderBackend() = default;

	virtual void init(Ref<Window> p_window) = 0;
	virtual void shutdown() = 0;

	// Device

	virtual void device_wait() = 0;

	virtual CommandQueue queue_get(QueueType p_type) = 0;

	// Buffer

	virtual Buffer buffer_create(uint64_t p_size,
			BitField<BufferUsageBits> p_usage,
			MemoryAllocationType p_allocation_type) = 0;

	virtual void buffer_free(Buffer p_buffer) = 0;

	virtual BufferDeviceAddress buffer_get_device_address(Buffer p_buffer) = 0;

	virtual uint8_t* buffer_map(Buffer p_buffer) = 0;

	virtual void buffer_unmap(Buffer p_buffer) = 0;

	// Image

	virtual Image image_create(DataFormat p_format, Vec2u p_size,
			const void* p_data = nullptr,
			BitField<ImageUsageBits> p_usage = IMAGE_USAGE_SAMPLED_BIT,
			bool p_mipmapped = false) = 0;

	virtual void image_free(Image p_image) = 0;

	virtual Vec3u image_get_size(Image p_image) = 0;

	virtual Sampler sampler_create(
			ImageFiltering p_min_filter = IMAGE_FILTERING_LINEAR,
			ImageFiltering p_mag_filter = IMAGE_FILTERING_LINEAR,
			ImageWrappingMode p_wrap_u = IMAGE_WRAPPING_MODE_CLAMP_TO_EDGE,
			ImageWrappingMode p_wrap_v = IMAGE_WRAPPING_MODE_CLAMP_TO_EDGE,
			ImageWrappingMode p_wrap_w = IMAGE_WRAPPING_MODE_CLAMP_TO_EDGE) = 0;

	virtual void sampler_free(Sampler p_sampler) = 0;

	// Shader

	virtual Shader shader_create_from_bytecode(
			const std::vector<SpirvData>& p_shaders) = 0;

	virtual void shader_free(Shader p_shader) = 0;

	virtual std::vector<ShaderInterfaceVariable> shader_get_vertex_inputs(
			Shader p_shader) = 0;

	// Swapchain

	// not valid until resized
	virtual Swapchain swapchain_create() = 0;

	virtual void swapchain_resize(
			CommandQueue p_cmd_queue, Swapchain p_swapchain, Vec2u size) = 0;

	/**
	 * @returns `Image` if succeed `{}` if resize needed
	 */
	virtual Optional<Image> swapchain_acquire_image(
			Swapchain p_swapchain, Semaphore p_semaphore) = 0;

	virtual Vec2u swapchain_get_extent(Swapchain p_swapchain) = 0;

	virtual DataFormat swapchain_get_format(Swapchain p_swapchain) = 0;

	virtual void swapchain_free(Swapchain p_swapchain) = 0;

	// UniformSet

	virtual UniformSet uniform_set_create(VectorView<ShaderUniform> p_uniforms,
			Shader p_shader, uint32_t p_set_index) = 0;

	virtual void uniform_set_free(UniformSet p_uniform_set) = 0;

	// Sync

	virtual Fence fence_create() = 0;

	virtual void fence_free(Fence p_fence) = 0;

	virtual void fence_wait(Fence p_fence) = 0;

	virtual void fence_reset(Fence p_fence) = 0;

	virtual Semaphore semaphore_create() = 0;

	virtual void semaphore_free(Semaphore p_semaphore) = 0;

	// Pipeline

	virtual Pipeline render_pipeline_create(Shader p_shader,
			RenderPrimitive p_render_primitive,
			PipelineVertexInputState p_input_state,
			PipelineRasterizationState p_rasterization_state,
			PipelineMultisampleState p_multisample_state,
			PipelineDepthStencilState p_depth_stencil_state,
			PipelineColorBlendState p_blend_state,
			BitField<PipelineDynamicStateFlags> p_dynamic_state,
			RenderingState p_rendering_state) = 0;

	virtual Pipeline compute_pipeline_create(Shader p_shader) = 0;

	virtual void pipeline_free(Pipeline p_pipeline) = 0;

	// Command Queue

	virtual void queue_submit(CommandQueue p_queue, CommandBuffer p_cmd,
			Fence p_fence = GL_NULL_HANDLE,
			Semaphore p_wait_semaphore = GL_NULL_HANDLE,
			Semaphore p_signal_semaphore = GL_NULL_HANDLE) = 0;

	// returns `true` if succeed `false` if resize needed
	virtual bool queue_present(CommandQueue p_queue, Swapchain p_swapchain,
			Semaphore p_wait_semaphore = GL_NULL_HANDLE) = 0;

	// Commands

	virtual void command_immediate_submit(
			std::function<void(CommandBuffer p_cmd)>&& p_function) = 0;

	virtual CommandPool command_pool_create(CommandQueue p_queue) = 0;

	virtual void command_pool_free(CommandPool p_command_pool) = 0;

	virtual CommandBuffer command_pool_allocate(CommandPool p_command_pool) = 0;

	virtual std::vector<CommandBuffer> command_pool_allocate(
			CommandPool p_command_pool, const uint32_t count) = 0;

	virtual void command_pool_reset(CommandPool p_command_pool) = 0;

	virtual void command_begin(CommandBuffer p_cmd) = 0;

	virtual void command_end(CommandBuffer p_cmd) = 0;

	virtual void command_reset(CommandBuffer p_cmd) = 0;

	virtual void command_begin_rendering(CommandBuffer p_cmd,
			const Vec2u& p_draw_extent, VectorView<Image> p_color_attachments,
			Image p_depth_attachment = GL_NULL_HANDLE) = 0;

	virtual void command_end_rendering(CommandBuffer p_cmd) = 0;

	// image layout must be IMAGE_LAYOUT_GENERAL
	virtual void command_clear_color(CommandBuffer p_cmd, Image p_image,
			const Color& p_clear_color,
			ImageAspectFlags p_image_aspect = IMAGE_ASPECT_COLOR_BIT) = 0;

	virtual void command_bind_graphics_pipeline(
			CommandBuffer p_cmd, Pipeline p_pipeline) = 0;

	virtual void command_bind_compute_pipeline(
			CommandBuffer p_cmd, Pipeline p_pipeline) = 0;

	virtual void command_bind_vertex_buffers(CommandBuffer p_cmd,
			uint32_t p_first_binding, std::vector<Buffer> p_vertex_buffers,
			std::vector<uint64_t> p_offsets) = 0;

	virtual void command_bind_index_buffer(CommandBuffer p_cmd,
			Buffer p_index_buffer, uint64_t p_offset,
			IndexType p_index_type) = 0;

	virtual void command_draw(CommandBuffer p_cmd, uint32_t p_vertex_count,
			uint32_t p_instance_count = 1, uint32_t p_first_vertex = 0,
			uint32_t p_first_instance = 0) = 0;

	virtual void command_draw_indexed(CommandBuffer p_cmd,
			uint32_t p_index_count, uint32_t p_instance_count = 1,
			uint32_t p_first_index = 0, int32_t p_vertex_offset = 0,
			uint32_t p_first_instance = 0) = 0;

	virtual void command_draw_indexed_indirect(CommandBuffer p_cmd,
			Buffer p_buffer, uint64_t p_offset, uint32_t p_draw_count,
			uint32_t p_stride) = 0;

	virtual void command_dispatch(CommandBuffer p_cmd, uint32_t p_group_count_x,
			uint32_t p_group_count_y, uint32_t p_group_count_z) = 0;

	virtual void command_bind_uniform_sets(CommandBuffer p_cmd, Shader p_shader,
			uint32_t p_first_set, VectorView<UniformSet> p_uniform_sets,
			PipelineType p_type = PIPELINE_TYPE_GRAPHICS) = 0;

	virtual void command_push_constants(CommandBuffer p_cmd, Shader p_shader,
			uint64_t p_offset, uint32_t p_size,
			const void* p_push_constants) = 0;

	virtual void command_set_viewport(
			CommandBuffer p_cmd, const Vec2u& size) = 0;

	virtual void command_set_scissor(CommandBuffer p_cmd, const Vec2u& p_size,
			const Vec2u& p_offset = { 0, 0 }) = 0;

	// NOTE: dynamic state must be enabled for this
	virtual void command_set_depth_bias(CommandBuffer p_cmd,
			float p_depth_bias_constant_factor, float p_depth_bias_clamp,
			float p_depth_bias_slope_factor) = 0;

	virtual void command_copy_buffer(CommandBuffer p_cmd, Buffer p_src_buffer,
			Buffer p_dst_buffer, VectorView<BufferCopyRegion> p_regions) = 0;

	// image layout must be VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
	virtual void command_copy_buffer_to_image(CommandBuffer p_cmd,
			Buffer p_src_buffer, Image p_dst_image,
			VectorView<BufferImageCopyRegion> p_regions) = 0;

	virtual void command_copy_image_to_image(CommandBuffer p_cmd,
			Image p_src_image, Image p_dst_image, const Vec2u& p_src_extent,
			const Vec2u& p_dst_extent) = 0;

	virtual void command_transition_image(CommandBuffer p_cmd, Image p_image,
			ImageLayout p_current_layout, ImageLayout p_new_layout) = 0;

	// ImGui

	virtual void imgui_init_for_platform(GLFWwindow* p_glfw_window) = 0;

	virtual void imgui_render_for_platform(CommandBuffer p_cmd) = 0;

	virtual void imgui_new_frame_for_platform() = 0;

	virtual void* imgui_image_upload(Image p_image, Sampler p_sampler) = 0;

	virtual void imgui_image_free(void* p_set) = 0;
};
