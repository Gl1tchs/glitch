#pragma once

#include "gl/core/window.h"

#include "gl/renderer/scene_graph.h"

enum class RenderBackend {
	Vulkan,
	None,
};

[[nodiscard]] RenderBackend find_proper_backend() noexcept;

struct RendererSettings {
	float render_scale = 1.0f;
	// bool msaa;
};

class Renderer {
public:
	virtual ~Renderer() = default;

	// record drawing commands and submit them
	virtual void wait_and_render() = 0;

	// wait for all operations to finish
	virtual void wait_for_device() = 0;

	SceneGraph& get_scene_graph();

	RendererSettings& get_settings();

	static RenderBackend get_backend();

	static Ref<Renderer> create(RenderBackend backend, Ref<Window> window);

protected:
	/**
	 * @brief destroys scene graph with it's dependencies,
	 * this function must be called before the uninitialization
	 * state of the graphics API
	 */
	void _destroy_scene_graph();

private:
	SceneGraph scene_graph;

	RendererSettings settings{};
};
