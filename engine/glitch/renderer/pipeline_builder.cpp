#include "glitch/renderer/pipeline_builder.h"

#include "glitch/renderer/render_backend.h"
#include "glitch/renderer/renderer.h"

namespace gl {

PipelineBuilder::PipelineBuilder() {
	primitive_type = RenderPrimitive::TRIANGLE_LIST;
	vertex_input = {};
	rasterization = {};
	multisample = {};
	depth_stencil_state = {};
	color_blend_state = PipelineColorBlendState::create_disabled();
	rendering_state = {};
}

PipelineBuilder& PipelineBuilder::add_color_attachment(DataFormat p_format) {
	rendering_state.color_attachments.push_back(p_format);
	return *this;
}

PipelineBuilder& PipelineBuilder::set_depth_attachment(Optional<DataFormat> p_format) {
	if (p_format) {
		rendering_state.depth_attachment = *p_format;
	}
	return *this;
}

PipelineBuilder& PipelineBuilder::add_shader_stage(
		ShaderStage p_stage, const std::vector<uint32_t>& p_spirv_data) {
	SpirvData shader_data = {};
	shader_data.stage = p_stage;
	shader_data.byte_code = p_spirv_data;

	shader_stages.push_back(shader_data);

	return *this;
}

PipelineBuilder& PipelineBuilder::with_depth_test(CompareOperator p_op, bool p_depth_write) {
	depth_stencil_state.depth_compare_operator = CompareOperator::LESS;
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
		uint32_t p_samples, bool p_enable_sample_shading) {
	const uint32_t max_sample_count = Renderer::get_backend()->get_max_msaa_samples();

	if ((p_samples != 1 && p_samples % 2 != 0) || p_samples > max_sample_count) {
		GL_LOG_ERROR(
				"[PipelineBuilder::with_multisample] Invalid MSAA sample count: {}. Must be 1 or "
				"power-of-two, and ≤ {}",
				p_samples, max_sample_count);
		return *this;
	}

	multisample.sample_count = p_samples;
	if (p_enable_sample_shading) {
		multisample.enable_sample_shading = true;
		multisample.min_sample_shading = 0.2f;
	}

	return *this;
}

PipelineBuilder& PipelineBuilder::set_render_primitive(RenderPrimitive p_prim) {
	primitive_type = p_prim;

	return *this;
}

std::pair<Shader, Pipeline> PipelineBuilder::build(RenderPass p_render_pass) {
	Ref<RenderBackend> backend = Renderer::get_backend();

	Shader shader = backend->shader_create_from_bytecode(shader_stages);

	// If any render pass provided use it to build the pipeline otherwise get
	// the default render pass from rendering device1
	Pipeline pipeline = backend->render_pipeline_create(shader, primitive_type, vertex_input,
			rasterization, multisample, depth_stencil_state, color_blend_state, 0, rendering_state);

	return std::make_pair(shader, pipeline);
}

} //namespace gl