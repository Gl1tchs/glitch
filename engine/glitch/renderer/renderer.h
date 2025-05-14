/**
 * @file renderer.h
 */

#pragma once

#include "glitch/core/window.h"

#include "glitch/renderer/types.h"

class RenderBackend;

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

/**
 * Class representing the main renderer that is responsible of keeping
 * cpu/gpu communication stable and making sure our drawing commands
 * are being submitted and presented.
 */
class GL_API Renderer {
public:
	Renderer(Ref<Window> p_window);
	~Renderer();

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

	glm::uvec2 get_draw_extent() { return draw_extent; }

	Image get_draw_image() { return draw_image; }

	Image get_depth_image() { return depth_image; }

	RenderStats& get_stats() { return stats; }

	static DataFormat get_draw_image_format() {
		return s_instance->draw_image_format;
	}

	static DataFormat get_depth_image_format() {
		return s_instance->depth_image_format;
	}

	static Ref<RenderBackend> get_backend() { return s_instance->backend; }

private:
	void _geometry_pass(CommandBuffer p_cmd);

	void _imgui_pass(CommandBuffer p_cmd, Image p_target_image);

private:
	void _imgui_init();

	void _request_resize();

	void _reset_stats();

	inline FrameData& _get_current_frame() {
		return frames[frame_number % SWAPCHAIN_BUFFER_SIZE];
	};

private:
	static Renderer* s_instance;

	Ref<Window> window;

	// drawing data
	Ref<RenderBackend> backend;

	CommandQueue graphics_queue;
	CommandQueue present_queue;

	Swapchain swapchain;

	glm::uvec2 draw_extent;

	static constexpr uint8_t SWAPCHAIN_BUFFER_SIZE = 2;
	FrameData frames[SWAPCHAIN_BUFFER_SIZE];

	uint32_t frame_number = 0;

	Image current_swapchain_image;

	Image draw_image;
	const DataFormat draw_image_format = DATA_FORMAT_R16G16B16A16_SFLOAT;

	Image depth_image;
	const DataFormat depth_image_format = DATA_FORMAT_D32_SFLOAT;

	// imgui data
	bool imgui_being_used = false;

	// misc
	RenderStats stats = {};
};
