#include "glitch/scene/scene_renderer.h"

#include "glitch/core/application.h"
#include "glitch/renderer/material_definitions.h"
#include "glitch/renderer/types.h"

namespace gl {

SceneRenderer::SceneRenderer(const SceneRendererSpecification& p_specs) :
		renderer(Application::get_instance()->get_renderer()),
		backend(renderer->get_backend()) {
	renderer->set_msaa_samples(p_specs.msaa);

	// Create and initialize graphics passes
	clear_pass = create_ref<ClearPass>();
	renderer->add_pass(clear_pass, -10);

	mesh_pass = create_ref<MeshPass>();
	renderer->add_pass(mesh_pass);

	DataFormat color_attachment_format = backend->image_get_format(
			renderer->get_render_image("geo_albedo").value());
	DataFormat depth_attachment_format = backend->image_get_format(
			renderer->get_render_image("geo_depth").value());

	// Register material definitions
	MaterialSystem::init();
	MaterialSystem::register_definition("unlit_standard",
			get_unlit_standard_definition(renderer->get_msaa_samples(),
					color_attachment_format, depth_attachment_format));
	MaterialSystem::register_definition("pbr_standard",
			get_pbr_standard_definition(renderer->get_msaa_samples(),
					color_attachment_format, depth_attachment_format));
}

SceneRenderer::~SceneRenderer() { renderer->wait_for_device(); }

void SceneRenderer::submit(const DrawingContext& p_ctx) {
	GL_PROFILE_SCOPE;

	if (!p_ctx.scene) {
		GL_LOG_WARNING("No Scene assigned to render!");
		return;
	}

	mesh_pass->set_scene(p_ctx.scene);

	renderer->set_render_present_mode(false);
	renderer->set_resolution_scale(p_ctx.settings.resolution_scale);
	renderer->set_vsync(p_ctx.settings.vsync);

	CommandBuffer cmd = renderer->begin_render();
	{
		renderer->execute(cmd);
	}
	renderer->end_render();
}

void SceneRenderer::submit_func(RenderFunc&& p_func) {
	render_funcs.push_back(p_func);
}

} //namespace gl