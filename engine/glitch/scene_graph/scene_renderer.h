/**
 * @file scene_renderer.h
 */

#pragma once

#include "glitch/renderer/camera.h"
#include "glitch/renderer/renderer.h"
#include "glitch/renderer/storage_buffer.h"
#include "glitch/scene_graph/scene_graph.h"

namespace gl {

struct SceneData {
	glm::mat4 view_projection;
	glm::vec3 camera_position;
};

struct PushConstants {
	BufferDeviceAddress vertex_buffer;
	BufferDeviceAddress scene_buffer;
	glm::mat4 transform;
};

struct DrawingContext {
	SceneGraph* scene_graph;

	PerspectiveCamera camera;

	RendererSettings settings = {};
};

struct SceneRendererSpecification {
	uint32_t msaa = 1;
};

/**
 * High level rendering interface
 */
class GL_API SceneRenderer {
public:
	using RenderFunc = std::function<void(CommandBuffer)>;

	SceneRenderer(const SceneRendererSpecification& p_specs);
	~SceneRenderer();

	void submit(const DrawingContext& p_ctx);

	/**
	 * Push a rendering function into stack using a render pass
	 */
	void submit_func(RenderFunc&& p_func);

private:
	RenderQueue _preprocess_render(const DrawingContext& p_ctx);

	void _geometry_pass(CommandBuffer p_cmd, const RenderQueue& p_render_queue);

private:
	Ref<Renderer> renderer;
	Ref<RenderBackend> backend;

	PushConstants push_constants = {};

	PerspectiveCamera camera;

	SceneData scene_data;
	size_t scene_data_hash;
	Ref<StorageBuffer> scene_data_sbo;

	std::vector<RenderFunc> render_funcs;
};

} //namespace gl