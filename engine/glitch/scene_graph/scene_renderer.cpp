#include "glitch/scene_graph/scene_renderer.h"

#include "glitch/core/application.h"
#include "glitch/renderer/material_definitions.h"
#include "glitch/renderer/types.h"

namespace gl {

SceneRenderer::SceneRenderer(const SceneRendererSpecification& p_specs) :
		renderer(Application::get_instance()->get_renderer()),
		backend(renderer->get_backend()) {
	renderer->set_msaa_samples(p_specs.msaa);

	// Create and initialize mesh pass
	mesh_pass = create_ref<MeshPass>();
	renderer->add_pass(mesh_pass);

	DataFormat color_attachment_format = backend->image_get_format(
			renderer->get_render_image("geo_albedo").value());
	DataFormat depth_attachment_format = backend->image_get_format(
			renderer->get_render_image("geo_depth").value());

	// Register material definitions
	MaterialSystem::init();
	MaterialSystem::register_definition("unlit_standart",
			get_unlit_material_definition(renderer->get_msaa_samples(),
					color_attachment_format, depth_attachment_format));
	MaterialSystem::register_definition("urp_standart",
			get_urp_material_definition(renderer->get_msaa_samples(),
					color_attachment_format, depth_attachment_format));
}

SceneRenderer::~SceneRenderer() { renderer->wait_for_device(); }

void SceneRenderer::submit(const DrawingContext& p_ctx) {
	GL_PROFILE_SCOPE;

	if (!p_ctx.scene_graph) {
		GL_LOG_WARNING("No SceneGraph assigned to render!");
		return;
	}

	mesh_pass->set_camera(p_ctx.camera);
	mesh_pass->set_scene_graph(p_ctx.scene_graph);

	renderer->set_render_present_mode(false);
	renderer->set_resolution_scale(p_ctx.settings.resolution_scale);

	CommandBuffer cmd = renderer->begin_render();
	{
		renderer->execute(cmd);
	}
	renderer->end_render();
}

void SceneRenderer::submit_func(RenderFunc&& p_func) {
	render_funcs.push_back(p_func);
}

void SceneRenderer::_geometry_pass(
		CommandBuffer p_cmd, const RenderQueue& p_render_queue) {
	if (p_render_queue.empty()) {
		return;
	}

	// TODO: this should probably has their own render pass and a priority
	// parameter should be given
	for (auto& render_func : render_funcs) {
		render_func(p_cmd);
	}
	render_funcs.clear();
}

} //namespace gl