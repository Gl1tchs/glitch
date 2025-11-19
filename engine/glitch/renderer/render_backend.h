/**
 * @file render_backend.h
 */

#pragma once

#include "glitch/core/color.h"
#include "glitch/core/templates/bit_field.h"
#include "glitch/core/templates/vector_view.h"
#include "glitch/core/window.h"
#include "glitch/renderer/types.h"

namespace gl {

enum class SwapchainAcquireError {
	OUT_OF_DATE, // resize needed
	ERROR,
};

/**
 * Abstract class who is responsible of communicating with GPU
 */
class GL_API RenderBackend {
public:
	virtual ~RenderBackend() = default;

	virtual void init(std::shared_ptr<Window> p_window) = 0;
	virtual void shutdown() = 0;

	// Device

	virtual void device_wait() = 0;

	virtual CommandQueue queue_get(QueueType p_type) = 0;

	virtual uint32_t get_max_msaa_samples() const = 0;

	// Buffer

	virtual Buffer buffer_create(uint64_t p_size, BitField<BufferUsageBits> p_usage,
			MemoryAllocationType p_allocation_type) = 0;

	virtual void buffer_free(Buffer p_buffer) = 0;

	virtual BufferDeviceAddress buffer_get_device_address(Buffer p_buffer) = 0;

	virtual uint8_t* buffer_map(Buffer p_buffer) = 0;

	virtual void buffer_unmap(Buffer p_buffer) = 0;

	// Image

	virtual Image image_create(DataFormat p_format, glm::uvec2 p_size, const void* p_data = nullptr,
			BitField<ImageUsageBits> p_usage = IMAGE_USAGE_SAMPLED_BIT, bool p_mipmapped = false,
			uint32_t p_samples = 1) = 0;

	virtual void image_free(Image p_image) = 0;

	virtual glm::uvec3 image_get_size(Image p_image) = 0;

	virtual DataFormat image_get_format(Image p_image) = 0;

	virtual uint32_t image_get_mip_levels(Image p_image) = 0;

	virtual Sampler sampler_create(ImageFiltering p_min_filter = ImageFiltering::LINEAR,
			ImageFiltering p_mag_filter = ImageFiltering::LINEAR,
			ImageWrappingMode p_wrap_u = ImageWrappingMode::CLAMP_TO_EDGE,
			ImageWrappingMode p_wrap_v = ImageWrappingMode::CLAMP_TO_EDGE,
			ImageWrappingMode p_wrap_w = ImageWrappingMode::CLAMP_TO_EDGE,
			uint32_t p_mip_levels = 0) = 0;

	virtual void sampler_free(Sampler p_sampler) = 0;

	// Shader

	virtual Shader shader_create_from_bytecode(const std::vector<SpirvData>& p_shaders) = 0;

	virtual void shader_free(Shader p_shader) = 0;

	virtual std::vector<ShaderInterfaceVariable> shader_get_vertex_inputs(Shader p_shader) = 0;

	// Render pass

	virtual RenderPass render_pass_create(VectorView<RenderPassAttachment> p_attachments,
			VectorView<SubpassInfo> p_subpasses) = 0;

	virtual void render_pass_destroy(RenderPass p_render_pass) = 0;

	// Frame Buffer

	virtual FrameBuffer frame_buffer_create(RenderPass p_render_pass,
			VectorView<Image> p_attachments, const glm::uvec2& p_extent) = 0;

	virtual void frame_buffer_destroy(FrameBuffer p_frame_buffer) = 0;

	// Swapchain

	// not valid until resized
	virtual Swapchain swapchain_create() = 0;

	virtual void swapchain_resize(CommandQueue p_cmd_queue, Swapchain p_swapchain, glm::uvec2 size,
			bool p_vsync = false) = 0;

	virtual size_t swapchain_get_image_count(Swapchain p_swapchain) = 0;

	virtual std::vector<Image> swapchain_get_images(Swapchain p_swapchain) = 0;

	/**
	 * @returns `Image` if succeed `nullopt` if resize needed
	 */
	virtual Result<Image, SwapchainAcquireError> swapchain_acquire_image(
			Swapchain p_swapchain, Semaphore p_semaphore, uint32_t* o_image_index = nullptr) = 0;

	virtual glm::uvec2 swapchain_get_extent(Swapchain p_swapchain) = 0;

	virtual DataFormat swapchain_get_format(Swapchain p_swapchain) = 0;

	virtual void swapchain_free(Swapchain p_swapchain) = 0;

	// UniformSet

	virtual UniformSet uniform_set_create(
			VectorView<ShaderUniform> p_uniforms, Shader p_shader, uint32_t p_set_index) = 0;

	virtual void uniform_set_free(UniformSet p_uniform_set) = 0;

	// Sync

	virtual Fence fence_create() = 0;

	virtual void fence_free(Fence p_fence) = 0;

	virtual void fence_wait(Fence p_fence) = 0;

	virtual void fence_reset(Fence p_fence) = 0;

	virtual Semaphore semaphore_create() = 0;

	virtual void semaphore_free(Semaphore p_semaphore) = 0;

	// Pipeline

	virtual Pipeline render_pipeline_create(Shader p_shader, RenderPrimitive p_render_primitive,
			PipelineVertexInputState p_input_state,
			PipelineRasterizationState p_rasterization_state,
			PipelineMultisampleState p_multisample_state,
			PipelineDepthStencilState p_depth_stencil_state, PipelineColorBlendState p_blend_state,
			BitField<PipelineDynamicStateFlags> p_dynamic_state,
			RenderingState p_rendering_state) = 0;

	virtual Pipeline render_pipeline_create(Shader p_shader, RenderPass p_render_pass,
			RenderPrimitive p_render_primitive, PipelineVertexInputState p_vertex_input_state,
			PipelineRasterizationState p_rasterization_state,
			PipelineMultisampleState p_multisample_state,
			PipelineDepthStencilState p_depth_stencil_state, PipelineColorBlendState p_blend_state,
			BitField<PipelineDynamicStateFlags> p_dynamic_state) = 0;

	virtual Pipeline compute_pipeline_create(Shader p_shader) = 0;

	virtual void pipeline_free(Pipeline p_pipeline) = 0;

	// Command Queue

	virtual void queue_submit(CommandQueue p_queue, CommandBuffer p_cmd,
			Fence p_fence = GL_NULL_HANDLE, Semaphore p_wait_semaphore = GL_NULL_HANDLE,
			Semaphore p_signal_semaphore = GL_NULL_HANDLE) = 0;

	// returns `true` if succeed `false` if resize needed
	virtual bool queue_present(CommandQueue p_queue, Swapchain p_swapchain,
			Semaphore p_wait_semaphore = GL_NULL_HANDLE) = 0;

	// Commands

	virtual void command_immediate_submit(std::function<void(CommandBuffer p_cmd)>&& p_function,
			QueueType p_queue_type = QueueType::TRANSFER) = 0;

	virtual CommandPool command_pool_create(CommandQueue p_queue) = 0;

	virtual void command_pool_free(CommandPool p_command_pool) = 0;

	virtual CommandBuffer command_pool_allocate(CommandPool p_command_pool) = 0;

	virtual std::vector<CommandBuffer> command_pool_allocate(
			CommandPool p_command_pool, const uint32_t count) = 0;

	virtual void command_pool_reset(CommandPool p_command_pool) = 0;

	virtual void command_begin(CommandBuffer p_cmd) = 0;

	virtual void command_end(CommandBuffer p_cmd) = 0;

	virtual void command_reset(CommandBuffer p_cmd) = 0;

	virtual void command_begin_render_pass(CommandBuffer p_cmd, RenderPass p_render_pass,
			FrameBuffer framebuffer, const glm::uvec2& p_draw_extent,
			Color clear_color = COLOR_GRAY) = 0;

	virtual void command_end_render_pass(CommandBuffer p_cmd) = 0;

	virtual void command_begin_rendering(CommandBuffer p_cmd, const glm::uvec2& p_draw_extent,
			VectorView<RenderingAttachment> p_color_attachments,
			Image p_depth_attachment = GL_NULL_HANDLE) = 0;

	virtual void command_end_rendering(CommandBuffer p_cmd) = 0;

	// image layout must be ImageLayout::GENERAL
	virtual void command_clear_color(CommandBuffer p_cmd, Image p_image, const Color& p_clear_color,
			ImageAspectFlags p_image_aspect = IMAGE_ASPECT_COLOR_BIT) = 0;

	virtual void command_bind_graphics_pipeline(CommandBuffer p_cmd, Pipeline p_pipeline) = 0;

	virtual void command_bind_compute_pipeline(CommandBuffer p_cmd, Pipeline p_pipeline) = 0;

	virtual void command_bind_vertex_buffers(CommandBuffer p_cmd, uint32_t p_first_binding,
			std::vector<Buffer> p_vertex_buffers, std::vector<uint64_t> p_offsets) = 0;

	virtual void command_bind_index_buffer(CommandBuffer p_cmd, Buffer p_index_buffer,
			uint64_t p_offset, IndexType p_index_type) = 0;

	virtual void command_draw(CommandBuffer p_cmd, uint32_t p_vertex_count,
			uint32_t p_instance_count = 1, uint32_t p_first_vertex = 0,
			uint32_t p_first_instance = 0) = 0;

	virtual void command_draw_indexed(CommandBuffer p_cmd, uint32_t p_index_count,
			uint32_t p_instance_count = 1, uint32_t p_first_index = 0, int32_t p_vertex_offset = 0,
			uint32_t p_first_instance = 0) = 0;

	virtual void command_draw_indexed_indirect(CommandBuffer p_cmd, Buffer p_buffer,
			uint64_t p_offset, uint32_t p_draw_count, uint32_t p_stride) = 0;

	virtual void command_dispatch(CommandBuffer p_cmd, uint32_t p_group_count_x,
			uint32_t p_group_count_y, uint32_t p_group_count_z) = 0;

	virtual void command_bind_uniform_sets(CommandBuffer p_cmd, Shader p_shader,
			uint32_t p_first_set, VectorView<UniformSet> p_uniform_sets,
			PipelineType p_type = PipelineType::GRAPHICS) = 0;

	virtual void command_push_constants(CommandBuffer p_cmd, Shader p_shader, uint64_t p_offset,
			uint32_t p_size, const void* p_push_constants) = 0;

	virtual void command_set_viewport(CommandBuffer p_cmd, const glm::uvec2& size) = 0;

	virtual void command_set_scissor(CommandBuffer p_cmd, const glm::uvec2& p_size,
			const glm::uvec2& p_offset = { 0, 0 }) = 0;

	// NOTE: dynamic state must be enabled for this
	virtual void command_set_depth_bias(CommandBuffer p_cmd, float p_depth_bias_constant_factor,
			float p_depth_bias_clamp, float p_depth_bias_slope_factor) = 0;

	virtual void command_copy_buffer(CommandBuffer p_cmd, Buffer p_src_buffer, Buffer p_dst_buffer,
			VectorView<BufferCopyRegion> p_regions) = 0;

	// image layout must be ImageLayout::TRANSFER_DST_OPTIMAL
	virtual void command_copy_buffer_to_image(CommandBuffer p_cmd, Buffer p_src_buffer,
			Image p_dst_image, VectorView<BufferImageCopyRegion> p_regions) = 0;

	virtual void command_copy_image_to_image(CommandBuffer p_cmd, Image p_src_image,
			Image p_dst_image, const glm::uvec2& p_src_extent, const glm::uvec2& p_dst_extent,
			uint32_t p_src_mip_level = 0, uint32_t p_dst_mip_level = 0) = 0;

	virtual void command_transition_image(CommandBuffer p_cmd, Image p_image,
			ImageLayout p_current_layout, ImageLayout p_new_layout, uint32_t p_base_mip_level = 0,
			uint32_t p_level_count = GL_REMAINING_MIP_LEVELS) = 0;

	// ImGui

	virtual void imgui_init_for_platform(GLFWwindow* p_glfw_window, DataFormat p_color_format) = 0;

	virtual void imgui_render_for_platform(CommandBuffer p_cmd) = 0;

	virtual void imgui_new_frame_for_platform() = 0;

	virtual void* imgui_image_upload(Image p_image, Sampler p_sampler) = 0;

	virtual void imgui_image_free(void* p_set) = 0;
};

} //namespace gl