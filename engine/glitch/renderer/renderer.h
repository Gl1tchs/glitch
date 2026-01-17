/**
 * @file renderer.h
 */

#pragma once

#include "glitch/core/window.h"
#include <glgpu/device.h>

namespace gl {

enum class GraphicsAPI {
	VULKAN,
};

struct RenderStats {
	uint32_t draw_calls;
	uint32_t triangle_count;
};

struct FrameData {
	CommandPool command_pool = GL_NULL_HANDLE;
	CommandBuffer command_buffer = GL_NULL_HANDLE;

	Semaphore image_available_semaphore = GL_NULL_HANDLE,
			  render_finished_semaphore = GL_NULL_HANDLE;
	Fence render_fence = GL_NULL_HANDLE;

	void init(Device* device, CommandQueue queue);
	void destroy(Device* device);
};

struct RendererSettings {
	float resolution_scale = 1.0f;
	bool vsync = false;
};

class GraphicsPass;

/**
 * Class representing the low level renderig interface that is responsible of
 * keeping cpu/gpu communication stable and making sure our drawing commands are
 * being submitted and presented.
 */
class GL_API Renderer {
public:
	Renderer(std::shared_ptr<Window> window, RendererSettings settings = {});
	~Renderer();

	/**
	 * Begin rendering context, reset state, do necessary image
	 * transactions.
	 */
	CommandBuffer begin_render();

	/**
	 * End rendering context, submit command buffer and present onto
	 * surface.
	 */
	void end_render();

	/**
	 * @param priority Sets priority of the pass execute order. Higher
	 * priority will put other passes behind.
	 */
	void add_pass(std::shared_ptr<GraphicsPass> pass, int priority = 0);

	// Execute render passes
	void execute(CommandBuffer cmd);

	// Start drawing
	void begin_rendering(CommandBuffer cmd, Image color_attachment, Image depth_attachment,
			std::optional<Color> clear_color = std::nullopt);

	// End drawing
	void end_rendering(CommandBuffer cmd);

	enum class ImageCreateError { NONE = 0, ID_EXISTS };

	Result<Image, ImageCreateError> create_render_image(
			const std::string& name, DataFormat format, uint32_t usage);

	std::optional<Image> get_render_image(const std::string& name);

	/**
	 * Set the swapchain target image to get blittet into.
	 * If set to "" will use the first color attachment available
	 */
	void set_swapchain_target(const std::string& name);

	// Wait for rendering device operations to finish
	void wait_for_device();

	/**
	 * Begin ImGui rendering context, all imgui functions
	 * must be runned inside of this scope and this operation is
	 * defined as 1 imgui frame.
	 */
	void imgui_begin();

	// Ends imgui rendering context
	void imgui_end();

	// Settings

	// Sets whether the renderer should present the image into swapchain or not.
	void set_render_present_mode(bool present_to_swapchain);

	float get_resolution_scale() const;
	void set_resolution_scale(float scale);

	bool get_vsync_enabled() const;
	void set_vsync(bool vsync);

	uint32_t get_msaa_samples() const;

	// Triggers resize and buffer recreation do not call this in begin_render /
	// end_render
	void set_msaa_samples(uint32_t samples);

	// Accessors

	Swapchain get_swapchain();

	Vec2u get_resolution_extent() const;

	// Get descriptor set of the final image to use with imgui image
	void* get_final_image_descriptor() const;

	Vec2u get_final_image_size() const;

	RenderStats& get_stats();

	static Device* get_device();

private:
	void _imgui_pass(CommandBuffer cmd, Image target_image);

private:
	void _imgui_init();

	void _request_resize();

	void _reset_stats();

	inline FrameData& _get_current_frame() {
		return _frames[_frame_number % SWAPCHAIN_BUFFER_SIZE];
	};

private:
private:
	std::shared_ptr<Window> _window;

	CommandQueue _graphics_queue = GL_NULL_HANDLE;
	CommandQueue _present_queue = GL_NULL_HANDLE;

	Swapchain _swapchain = GL_NULL_HANDLE;

	uint32_t _image_index = 0;
	Image _current_swapchain_image = nullptr;

	static constexpr uint8_t SWAPCHAIN_BUFFER_SIZE = 2;
	FrameData _frames[SWAPCHAIN_BUFFER_SIZE];

	Image _final_image = GL_NULL_HANDLE;
	void* _final_image_descriptor = GL_NULL_HANDLE;
	Sampler _default_sampler = GL_NULL_HANDLE;

	struct RenderImage {
		Image image;
		DataFormat format;
		uint32_t usage;
		bool is_depth_attachment;
	};

	std::unordered_map<std::string, RenderImage> _renderpass_images;
	std::string _swapchain_target_image_id = "";

	std::vector<std::pair<std::shared_ptr<GraphicsPass>, int>> _graphics_passes;

	// Settings
	bool _should_present_to_swapchain = true;
	uint32_t _msaa_samples = 1;
	RendererSettings _settings = {};

	RenderStats _stats = {};
	uint32_t _frame_number = 0;

	// imgui data
	bool _imgui_being_used = false;
};

} //namespace gl