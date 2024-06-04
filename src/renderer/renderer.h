#pragma once

#include "core/window.h"

#include "renderer/scene_graph.h"

class RenderBackend;

enum GraphicsAPI {
	GRAPHICS_API_VULKAN,
	GRAPHICS_API_MAX,
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

class Renderer {
public:
	Renderer(Ref<Window> window);
	~Renderer();

	void wait_and_render();

	SceneGraph& get_scene_graph();

	RendererSettings& get_settings();

	RendererStats& get_stats();

	static GraphicsAPI get_graphics_api();

private:
	/**
	 * @brief destroys scene graph with it's dependencies,
	 * this function must be called before the uninitialization
	 * state of the graphics API
	 */
	void _destroy_scene_graph();

private:
	SceneGraph scene_graph;

	RendererSettings settings{};
	RendererStats stats{};
};
