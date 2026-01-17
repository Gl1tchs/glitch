/**
 * @file scene_renderer.h
 */

#pragma once

#include "glitch/renderer/renderer.h"
#include "glitch/scene/passes/clear_pass.h"
#include "glitch/scene/passes/mesh_pass.h"
#include "glitch/scene/scene.h"

namespace gl {

constexpr const char* DEFINITION_PATH_UNLIT_STANDARD =
		"mem://MaterialDefinition/pipelines/unlit_standard";

constexpr const char* DEFINITION_PATH_PBR_STANDARD =
		"mem://MaterialDefinition/pipelines/pbr_standard";

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

	SceneRenderer(const SceneRendererSpecification& specs);
	~SceneRenderer();

	void submit(const DrawingContext& ctx);

	/**
	 * Push a rendering function into stack using a render pass
	 */
	void submit_func(RenderFunc&& func);

private:
	std::shared_ptr<Renderer> _renderer;
	Device* _device;

	std::shared_ptr<ClearPass> _clear_pass;
	std::shared_ptr<MeshPass> _mesh_pass;

	std::vector<RenderFunc> _render_funcs;
};

} //namespace gl