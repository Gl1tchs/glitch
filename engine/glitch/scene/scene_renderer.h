/**
 * @file scene_renderer.h
 */

#pragma once

#include "glitch/renderer/renderer.h"
#include "glitch/scene/passes/clear_pass.h"
#include "glitch/scene/passes/mesh_pass.h"
#include "glitch/scene/scene.h"

namespace gl {

struct DrawingContext {
	std::shared_ptr<Scene> scene;
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
	std::shared_ptr<Renderer> renderer;
	std::shared_ptr<RenderBackend> backend;

	std::shared_ptr<ClearPass> clear_pass;
	std::shared_ptr<MeshPass> mesh_pass;

	std::vector<RenderFunc> render_funcs;
};

} //namespace gl