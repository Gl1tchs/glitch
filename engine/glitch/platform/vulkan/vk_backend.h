#pragma once

#include "glitch/core/deletion_queue.h"
#include "glitch/core/templates/versatile_resource.h"

#include "glitch/renderer/render_backend.h"

#include "glitch/platform/vulkan/vk_common.h"
#include "glitch/renderer/types.h"

class VulkanRenderBackend : public RenderBackend {
public:
	~VulkanRenderBackend() = default;

	// Generic

	void init(Ref<Window> p_window) override;
	void shutdown() override;

	// Device

	void device_wait() override;

	// Buffer

	struct VulkanBuffer {
		VkBuffer vk_buffer;
		struct {
			VmaAllocation handle;
			uint64_t size = UINT64_MAX;
		} allocation;
		uint64_t size = 0;
		VkBufferView vk_view = VK_NULL_HANDLE;
	};

	Buffer buffer_create(uint64_t p_size, BitField<BufferUsageBits> p_usage,
			MemoryAllocationType p_allocation_type) override;

	void buffer_free(Buffer p_buffer) override;

	BufferDeviceAddress buffer_get_device_address(Buffer p_buffer) override;

	uint8_t* buffer_map(Buffer p_buffer) override;

	void buffer_unmap(Buffer p_buffer) override;

	// Image

	struct VulkanImage {
		VkImage vk_image;
		VkImageView vk_image_view;
		VmaAllocation allocation;
		VkExtent3D image_extent;
		VkFormat image_format;
	};

	Image image_create(DataFormat p_format, glm::uvec2 p_size,
			const void* p_data = nullptr,
			BitField<ImageUsageBits> p_usage = IMAGE_USAGE_SAMPLED_BIT,
			bool p_mipmapped = false) override;

	void image_free(Image p_image) override;

	glm::uvec3 image_get_size(Image p_image) override;

	Sampler sampler_create(ImageFiltering p_min_filter = IMAGE_FILTERING_LINEAR,
			ImageFiltering p_mag_filter = IMAGE_FILTERING_LINEAR,
			ImageWrappingMode p_wrap_u = IMAGE_WRAPPING_MODE_CLAMP_TO_EDGE,
			ImageWrappingMode p_wrap_v = IMAGE_WRAPPING_MODE_CLAMP_TO_EDGE,
			ImageWrappingMode p_wrap_w =
					IMAGE_WRAPPING_MODE_CLAMP_TO_EDGE) override;

	void sampler_free(Sampler p_sampler) override;

	// Shader

	struct VulkanShader {
		std::vector<VkPipelineShaderStageCreateInfo> stage_create_infos;
		uint32_t push_constant_stages = 0;
		std::vector<VkDescriptorSetLayout> descriptor_set_layouts;
		VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;

		std::vector<ShaderInterfaceVariable> vertex_input_variables;

		// hash of the vulkan shader object defined as the
		// combination of the names of the shaders with
		// the pipeline layout
		size_t shader_hash;
	};

	Shader shader_create_from_bytecode(
			const std::vector<SpirvData>& p_shaders) override;

	void shader_free(Shader p_shader) override;

	std::vector<ShaderInterfaceVariable> shader_get_vertex_inputs(
			Shader p_shader) override;

	// Swapchain

	struct VulkanSwapchain {
		VkSwapchainKHR vk_swapchain = VK_NULL_HANDLE;
		VkFormat format = VK_FORMAT_UNDEFINED;
		VkColorSpaceKHR color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		VkExtent2D extent;
		std::vector<VulkanImage> images;
		uint32_t image_index;
	};

	Swapchain swapchain_create() override;

	void swapchain_resize(CommandQueue p_cmd_queue, Swapchain p_swapchain,
			glm::uvec2 size) override;

	Optional<Image> swapchain_acquire_image(
			Swapchain p_swapchain, Semaphore p_semaphore) override;

	glm::uvec2 swapchain_get_extent(Swapchain p_swapchain) override;

	DataFormat swapchain_get_format(Swapchain p_swapchain) override;

	void swapchain_free(Swapchain p_swapchain) override;

	// UniformSet

	static const uint32_t MAX_UNIFORM_POOL_ELEMENT = 65535;

	struct DescriptorSetPoolKey {
		uint16_t uniform_type[UNIFORM_TYPE_MAX] = {};

		bool operator<(const DescriptorSetPoolKey& p_other) const {
			return memcmp(uniform_type, p_other.uniform_type,
						   sizeof(uniform_type)) < 0;
		}
	};

	using DescriptorSetPools = std::map<DescriptorSetPoolKey,
			std::unordered_map<VkDescriptorPool, uint32_t>>;

	struct VulkanUniformSet {
		VkDescriptorSet vk_descriptor_set = VK_NULL_HANDLE;
		VkDescriptorPool vk_descriptor_pool = VK_NULL_HANDLE;
		DescriptorSetPools::iterator pool_sets_it = {};
	};

	UniformSet uniform_set_create(VectorView<ShaderUniform> p_uniforms,
			Shader p_shader, uint32_t p_set_index) override;

	void uniform_set_free(UniformSet p_uniform_set) override;

	// Sync

	Fence fence_create() override;

	void fence_free(Fence p_fence) override;

	void fence_wait(Fence p_fence) override;

	void fence_reset(Fence p_fence) override;

	Semaphore semaphore_create() override;

	void semaphore_free(Semaphore p_semaphore) override;

	// Pipeline

	struct VulkanPipeline {
		VkPipeline vk_pipeline;
		VkPipelineCache vk_pipeline_cache;
		size_t shader_hash;
	};

	Pipeline render_pipeline_create(Shader p_shader,
			RenderPrimitive p_render_primitive,
			PipelineVertexInputState p_vertex_input_state,
			PipelineRasterizationState p_rasterization_state,
			PipelineMultisampleState p_multisample_state,
			PipelineDepthStencilState p_depth_stencil_state,
			PipelineColorBlendState p_blend_state,
			BitField<PipelineDynamicStateFlags> p_dynamic_state,
			RenderingState p_rendering_state) override;

	Pipeline compute_pipeline_create(Shader p_shader) override;

	void pipeline_free(Pipeline p_pipeline) override;

	// Command Queue

	struct VulkanQueue {
		VkQueue queue;
		uint32_t queue_family;
	};

	CommandQueue queue_get(QueueType p_type) override;

	void queue_submit(CommandQueue p_queue, CommandBuffer p_cmd,
			Fence p_fence = GL_NULL_HANDLE,
			Semaphore p_wait_semaphore = GL_NULL_HANDLE,
			Semaphore p_signal_semaphore = GL_NULL_HANDLE) override;

	bool queue_present(CommandQueue p_queue, Swapchain p_swapchain,
			Semaphore p_wait_semaphore = GL_NULL_HANDLE) override;

	// Commands

	void command_immediate_submit(
			std::function<void(CommandBuffer p_cmd)>&& p_function) override;

	CommandPool command_pool_create(CommandQueue p_queue) override;

	void command_pool_free(CommandPool p_command_pool) override;

	CommandBuffer command_pool_allocate(CommandPool p_command_pool) override;

	std::vector<CommandBuffer> command_pool_allocate(
			CommandPool p_command_pool, const uint32_t count) override;

	void command_pool_reset(CommandPool p_command_pool) override;

	void command_begin(CommandBuffer p_cmd) override;

	void command_end(CommandBuffer p_cmd) override;

	void command_reset(CommandBuffer p_cmd) override;

	void command_begin_rendering(CommandBuffer p_cmd,
			const glm::uvec2& p_draw_extent, VectorView<Image> p_color_attachments,
			Image p_depth_attachment = GL_NULL_HANDLE) override;

	void command_end_rendering(CommandBuffer p_cmd) override;

	// image layout must be IMAGE_LAYOUT_GENERAL
	void command_clear_color(CommandBuffer p_cmd, Image p_image,
			const Color& p_clear_color,
			ImageAspectFlags p_image_aspect = IMAGE_ASPECT_COLOR_BIT) override;

	void command_bind_graphics_pipeline(
			CommandBuffer p_cmd, Pipeline p_pipeline) override;

	void command_bind_compute_pipeline(
			CommandBuffer p_cmd, Pipeline p_pipeline) override;

	void command_bind_vertex_buffers(CommandBuffer p_cmd,
			uint32_t p_first_binding, std::vector<Buffer> p_vertex_buffers,
			std::vector<uint64_t> p_offsets) override;

	void command_bind_index_buffer(CommandBuffer p_cmd, Buffer p_index_buffer,
			uint64_t p_offset, IndexType p_index_type) override;

	void command_draw(CommandBuffer p_cmd, uint32_t p_vertex_count,
			uint32_t p_instance_count = 1, uint32_t p_first_vertex = 0,
			uint32_t p_first_instance = 0) override;

	void command_draw_indexed(CommandBuffer p_cmd, uint32_t p_index_count,
			uint32_t p_instance_count = 1, uint32_t p_first_index = 0,
			int32_t p_vertex_offset = 0,
			uint32_t p_first_instance = 0) override;

	void command_draw_indexed_indirect(CommandBuffer p_cmd, Buffer p_buffer,
			uint64_t p_offset, uint32_t p_draw_count,
			uint32_t p_stride) override;

	void command_dispatch(CommandBuffer p_cmd, uint32_t p_group_count_x,
			uint32_t p_group_count_y, uint32_t p_group_count_z) override;

	void command_bind_uniform_sets(CommandBuffer p_cmd, Shader p_shader,
			uint32_t p_first_set, VectorView<UniformSet> p_uniform_sets,
			PipelineType p_type = PIPELINE_TYPE_GRAPHICS) override;

	void command_push_constants(CommandBuffer p_cmd, Shader p_shader,
			uint64_t p_offset, uint32_t p_size,
			const void* p_push_constants) override;

	void command_set_viewport(CommandBuffer p_cmd, const glm::uvec2& size) override;

	void command_set_scissor(CommandBuffer p_cmd, const glm::uvec2& p_size,
			const glm::uvec2& p_offset = { 0, 0 }) override;

	void command_set_depth_bias(CommandBuffer p_cmd,
			float p_depth_bias_constant_factor, float p_depth_bias_clamp,
			float p_depth_bias_slope_factor) override;

	void command_copy_buffer(CommandBuffer p_cmd, Buffer p_src_buffer,
			Buffer p_dst_buffer,
			VectorView<BufferCopyRegion> p_regions) override;

	void command_copy_buffer_to_image(CommandBuffer p_cmd, Buffer p_src_buffer,
			Image p_dst_image,
			VectorView<BufferImageCopyRegion> p_regions) override;

	void command_copy_image_to_image(CommandBuffer p_cmd, Image p_src_image,
			Image p_dst_image, const glm::uvec2& p_src_extent,
			const glm::uvec2& p_dst_extent) override;

	void command_transition_image(CommandBuffer p_cmd, Image p_image,
			ImageLayout p_current_layout, ImageLayout p_new_layout) override;

	// ImGui

	void imgui_init_for_platform(GLFWwindow* p_glfw_window) override;

	void imgui_render_for_platform(CommandBuffer p_cmd) override;

	void imgui_new_frame_for_platform() override;

	void* imgui_image_upload(Image p_image, Sampler p_sampler) override;

	void imgui_image_free(void* p_set) override;

private:
	Image _image_create(VkFormat p_format, VkExtent3D p_size,
			BitField<VkImageUsageFlags> p_usage, bool p_mipmapped);

	void _swapchain_release(VulkanSwapchain* p_swapchain);

	VmaPool _find_or_create_small_allocs_pool(uint32_t p_mem_type_index);

	VkDescriptorPool _uniform_pool_find_or_create(
			const DescriptorSetPoolKey& p_key,
			DescriptorSetPools::iterator* r_pool_sets_it);

	void _uniform_pool_unreference(DescriptorSetPools::iterator p_pool_sets_it,
			VkDescriptorPool p_vk_descriptor_pool);

private:
	static VulkanRenderBackend* s_instance;

	using VersatileResource = VersatileResourceTemplate<VulkanBuffer,
			VulkanImage, VulkanShader, VulkanUniformSet, VulkanPipeline>;

	VkInstance instance;
	VkDevice device;

	VkPhysicalDevice physical_device;
	VkPhysicalDeviceProperties physical_device_properties;
	VkPhysicalDeviceFeatures physical_device_features;

	VkSurfaceKHR surface;
	VkDebugUtilsMessengerEXT debug_messenger;

	VulkanQueue graphics_queue;
	VulkanQueue present_queue;
	VulkanQueue command_queue;

	static const uint32_t SMALL_ALLOCATION_MAX_SIZE = 4096;

	VmaAllocator allocator = nullptr;
	std::unordered_map<uint32_t, VmaPool> small_allocs_pools;

	DescriptorSetPools descriptor_set_pools;

	PagedAllocator<VersatileResource> resources_allocator;

	// immediate commands
	Fence imm_fence;
	CommandBuffer imm_command_buffer;
	CommandPool imm_command_pool;

	DeletionQueue deletion_queue;
};
