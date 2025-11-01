#include "glitch/scene/passes/clear_pass.h"

namespace gl {

void ClearPass::setup(Renderer& p_renderer) {
	GL_PROFILE_SCOPE;

	// Create pass resources
	p_renderer.create_render_image("geo_albedo",
			Renderer::get_backend()->swapchain_get_format(
					p_renderer.get_swapchain()),
			IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
	p_renderer.create_render_image("geo_depth", DataFormat::D32_SFLOAT,
			IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

	// p_renderer.set_swapchain_target("geo_albedo");
}

void ClearPass::execute(CommandBuffer p_cmd, Renderer& p_renderer) {
	GL_PROFILE_SCOPE;

	Ref<RenderBackend> backend = p_renderer.get_backend();

	// Empty pass to clear the screen
	// TODO: environment map rendering
	p_renderer.begin_rendering(p_cmd,
			p_renderer.get_render_image("geo_albedo").value(),
			p_renderer.get_render_image("geo_depth").value(), COLOR_GRAY);

	p_renderer.end_rendering(p_cmd);
}

} //namespace gl
