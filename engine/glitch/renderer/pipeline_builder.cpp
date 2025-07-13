#include "glitch/renderer/pipeline_builder.h"

#include "glitch/core/application.h"
#include "glitch/renderer/render_backend.h"
#include "glitch/renderer/renderer.h"

PipelineBuilder::PipelineBuilder() {
	Ref<Renderer> device = Application::get_instance()->get_renderer();

	vertex_input = {};
	rasterization = {};
	multisample = {};
	depth_stencil_state = {};
	color_blend_state = PipelineColorBlendState::create_disabled();
	rendering_state = {};
	rendering_state.color_attachments.push_back(
			device->get_color_attachment_format());
	rendering_state.depth_attachment = device->get_depth_attachment_format();
}

PipelineBuilder& PipelineBuilder::add_shader_stage(
		ShaderStage p_stage, const std::vector<uint32_t>& p_spirv_data) {
	SpirvData shader_data = {};
	shader_data.stage = p_stage;
	shader_data.byte_code = p_spirv_data;

	shader_stages.push_back(shader_data);

	return *this;
}

PipelineBuilder& PipelineBuilder::with_depth_test(
		CompareOperator p_op, bool p_depth_write) {
	depth_stencil_state.depth_compare_operator = COMPARE_OP_LESS;
	depth_stencil_state.enable_depth_test = true;
	depth_stencil_state.enable_depth_write = p_depth_write;
	depth_stencil_state.enable_depth_range = true;

	return *this;
}

PipelineBuilder& PipelineBuilder::with_blend() {
	color_blend_state = PipelineColorBlendState::create_blend();

	return *this;
}

PipelineBuilder& PipelineBuilder::with_multisample(
		ImageSamples p_samples, bool p_enable_sample_shading) {
	multisample.sample_count = IMAGE_SAMPLES_8;
	if (p_enable_sample_shading) {
		multisample.enable_sample_shading = true;
		multisample.min_sample_shading = 0.2f;
	}

	return *this;
}

std::pair<Shader, Pipeline> PipelineBuilder::build(RenderPass p_render_pass) {
	Ref<RenderBackend> backend = Renderer::get_backend();

	Shader shader = backend->shader_create_from_bytecode(shader_stages);

	// If any render pass provided use it to build the pipeline otherwise get
	// the default render pass from rendering device1
	Pipeline pipeline =
			backend->render_pipeline_create(shader, RENDER_PRIMITIVE_TRIANGLES,
					vertex_input, rasterization, multisample,
					depth_stencil_state, color_blend_state, 0, rendering_state);

	return std::make_pair(shader, pipeline);
}