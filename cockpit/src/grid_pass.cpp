#include "grid_pass.h"

#include "glitch/renderer/pipeline_builder.h"
#include "glitch/renderer/shader_library.h"

GridPass::~GridPass() {
	Ref<RenderBackend> backend = Renderer::get_backend();

	backend->pipeline_free(grid_pipeline);
	backend->shader_free(grid_shader);
}

void GridPass::setup(Renderer& p_renderer) {
	Ref<RenderBackend> backend = p_renderer.get_backend();

	DataFormat color_attachment_format = backend->image_get_format(
			p_renderer.get_render_image("geo_albedo").value());
	DataFormat depth_attachment_format = backend->image_get_format(
			p_renderer.get_render_image("geo_depth").value());

	std::tie(grid_shader, grid_pipeline) =
			PipelineBuilder()
					.add_color_attachment(color_attachment_format)
					.set_depth_attachment(depth_attachment_format)
					.add_shader_stage(ShaderStage::VERTEX,
							ShaderLibrary::get_spirv_data(
									"build/testbed/shaders/"
									"infinite_grid.vert.spv"))
					.add_shader_stage(ShaderStage::FRAGMENT,
							ShaderLibrary::get_spirv_data(
									"build/testbed/shaders/"
									"infinite_grid.frag.spv"))
					.with_depth_test(
							CompareOperator::LESS, false) // without depth write
					.with_blend()
					.with_multisample(p_renderer.get_msaa_samples(), true)
					.build();
}

void GridPass::execute(CommandBuffer p_cmd, Renderer& p_renderer) {
	Ref<RenderBackend> backend = p_renderer.get_backend();

	p_renderer.begin_rendering(p_cmd,
			p_renderer.get_render_image("geo_albedo").value(),
			p_renderer.get_render_image("geo_depth").value(), COLOR_GRAY);

	backend->command_bind_graphics_pipeline(p_cmd, grid_pipeline);

	PushConstants push_constants;
	push_constants.view_proj =
			camera.get_projection_matrix() * camera.get_view_matrix();
	push_constants.camera_pos = camera.transform.position;
	push_constants.grid_size = 100.0f;

	backend->command_push_constants(
			p_cmd, grid_shader, 0, sizeof(PushConstants), &push_constants);

	backend->command_draw(p_cmd, 6);

	p_renderer.end_rendering(p_cmd);
}

void GridPass::set_camera(const PerspectiveCamera& p_camera) {
	camera = p_camera;
}
