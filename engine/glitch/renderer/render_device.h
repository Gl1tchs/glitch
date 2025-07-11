/**
 * @file renderer.h
 */

#pragma once

#include "glitch/core/window.h"
#include "glitch/renderer/render_backend.h"
#include "glitch/renderer/types.h"

enum GraphicsAPI {
	GRAPHICS_API_VULKAN,
};

[[nodiscard]] GL_API GraphicsAPI find_proper_api() noexcept;

struct RenderStats {
	uint32_t draw_calls;
	uint32_t triangle_count;
};

struct FrameData {
	CommandPool command_pool;
	CommandBuffer command_buffer;

	Semaphore image_available_semaphore, render_finished_semaphore;
	Fence render_fence;
};

struct RendererSettings {
	Color clear_color = COLOR_GRAY;
	float resolution_scale = 1.0f;
};

/**
 * Class representing the low level renderig interface that is responsible of
 * keeping cpu/gpu communication stable and making sure our drawing commands are
 * being submitted and presented.
 */
class GL_API RenderDevice {
public:
	RenderDevice(Ref<Window> p_window);
	~RenderDevice();

	/**
	 * Begin rendering context, reset state, do necessary image
	 * transactions.
	 * @note If you want to draw directly to `draw_image` using compute shaders
	 * you must transition the image layout from
	 * IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL to IMAGE_LAYOUT_GENERAL and after
	 * when you are done with it, you must retransition the layout into
	 * IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL.
	 */
	CommandBuffer begin_render();

	/**
	 * End rendering context, submit command buffer and present onto
	 * surface.
	 */
	void end_render();

	void clear_pass(CommandBuffer p_cmd, Color clear_color = COLOR_GRAY);

	void begin_rendering(CommandBuffer p_cmd, RendererSettings p_settings = {});

	void end_rendering(CommandBuffer p_cmd);

	/**
	 * Wait for rendering device operations to finish
	 */
	void wait_for_device();

	/**
	 * Begin ImGui rendering context, all imgui functions
	 * must be runned inside of this scope and this operation is
	 * defined as 1 imgui frame.
	 */
	void imgui_begin();

	/**
	 * Ends imgui rendering context
	 */
	void imgui_end();

	// Sets scaling setting from 0.0 to 1.0
	void set_render_scale(float p_scale);

	Swapchain get_swapchain();

	Image get_draw_image();

	Image get_depth_image();

	RenderStats& get_stats();

	static DataFormat get_color_attachment_format();

	static DataFormat get_depth_attachment_format();

	static Ref<RenderBackend> get_backend();

private:
	void _imgui_pass(CommandBuffer p_cmd, Image p_target_image);

private:
	void _imgui_init();

	void _request_resize();

	void _reset_stats();

	inline FrameData& _get_current_frame() {
		return frames[frame_number % SWAPCHAIN_BUFFER_SIZE];
	};

private:
	static RenderDevice* s_instance;

	Ref<Window> window;

	// drawing data
	Ref<RenderBackend> backend;

	CommandQueue graphics_queue;
	CommandQueue present_queue;

	Swapchain swapchain;

	uint32_t image_index = 0;
	Image current_swapchain_image = nullptr;

	static constexpr uint8_t SWAPCHAIN_BUFFER_SIZE = 2;
	FrameData frames[SWAPCHAIN_BUFFER_SIZE];

	const DataFormat color_attachment_format = DATA_FORMAT_R16G16B16A16_SFLOAT;
	const DataFormat depth_attachment_format = DATA_FORMAT_D32_SFLOAT;

	Image color_image = GL_NULL_HANDLE;
	Image depth_image = GL_NULL_HANDLE;

	float render_scale = 1.0f;

	RenderStats stats = {};
	uint32_t frame_number = 0;

	// imgui data
	bool imgui_being_used = false;
};
