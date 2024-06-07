#pragma once

#include "core/window.h"

#include "renderer/material.h"
#include "renderer/scene_graph.h"
#include "renderer/types.h"

class RenderBackend;

enum GraphicsAPI {
	GRAPHICS_API_VULKAN,
};

[[nodiscard]] GraphicsAPI find_proper_api() noexcept;

struct RendererSettings {
	float render_scale = 1.0f;
	// bool msaa;
};

struct RendererStats {
	uint32_t draw_calls;
	uint32_t triangle_count;
};

struct FrameData {
	CommandPool command_pool;
	CommandBuffer command_buffer;

	Semaphore image_available_semaphore, render_finished_semaphore;
	Fence render_fence;
};

class Renderer {
public:
	Renderer(Ref<Window> p_window);
	~Renderer();

	void wait_and_render();

	SceneGraph& get_scene_graph();

	RendererSettings& get_settings();

	RendererStats& get_stats();

	static GraphicsAPI get_graphics_api();

private:
	void _geometry_pass(CommandBuffer p_cmd);

private:
	void _request_resize();

	/**
	 * @brief destroys scene graph with it's dependencies,
	 * this function must be called before the uninitialization
	 * state of the graphics API
	 */
	void _destroy_scene_graph();

	inline FrameData& _get_current_frame() {
		return frames[frame_number % SWAPCHAIN_BUFFER_SIZE];
	};

private:
	Ref<Window> window;
	Ref<RenderBackend> backend;

	Context context;
	Swapchain swapchain;

	Image draw_image;
	Image depth_image;

	Vec2u draw_extent;

	static constexpr uint8_t SWAPCHAIN_BUFFER_SIZE = 2;

	FrameData frames[SWAPCHAIN_BUFFER_SIZE];
	uint32_t frame_number = 0;

	Ref<Material> material;

	SceneGraph scene_graph;

	RendererSettings settings = {};
	RendererStats stats = {};
};
