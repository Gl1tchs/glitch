#include "glitch/renderer/pipeline_builder.h"

#include "glitch/renderer/renderer.h"

namespace gl {

PipelineBuilder::PipelineBuilder() {
	_primitive_type = RenderPrimitive::TRIANGLE_LIST;
	_vertex_input = {};
	_rasterization = {};
	_multisample = {};
	_depth_stencil_state = {};
	_color_blend_state = PipelineColorBlendState::create_disabled();
	_rendering_state = {};
}

PipelineBuilder& PipelineBuilder::add_color_attachment(DataFormat format) {
	_rendering_state.color_attachments.push_back(format);
	return *this;
}

PipelineBuilder& PipelineBuilder::set_depth_attachment(std::optional<DataFormat> format) {
	if (format) {
		_rendering_state.depth_attachment = *format;
	}
	return *this;
}

PipelineBuilder& PipelineBuilder::add_shader_stage(
		ShaderStageFlags stage, const std::vector<uint32_t>& spirv_data) {
	SpirvEntry shader_data = {};
	shader_data.stage = stage;
	shader_data.byte_code = spirv_data;

	_shader_stages.push_back(shader_data);

	return *this;
}

PipelineBuilder& PipelineBuilder::with_depth_test(CompareOperator op, bool depth_write) {
	_depth_stencil_state.depth_compare_operator = CompareOperator::LESS;
	_depth_stencil_state.enable_depth_test = true;
	_depth_stencil_state.enable_depth_write = depth_write;
	_depth_stencil_state.enable_depth_range = true;

	return *this;
}

PipelineBuilder& PipelineBuilder::with_blend() {
	_color_blend_state = PipelineColorBlendState::create_blend();

	return *this;
}

PipelineBuilder& PipelineBuilder::with_multisample(uint32_t samples, bool enable_sample_shading) {
	const uint32_t max_sample_count = Renderer::get_device()->get_max_msaa_samples();

	if ((samples != 1 && samples % 2 != 0) || samples > max_sample_count) {
		GL_LOG_ERROR(
				"[PipelineBuilder::with_multisample] Invalid MSAA sample count: {}. Must be 1 or "
				"power-of-two, and ≤ {}",
				samples, max_sample_count);
		return *this;
	}

	_multisample.sample_count = samples;
	if (enable_sample_shading) {
		_multisample.enable_sample_shading = true;
		_multisample.min_sample_shading = 0.2f;
	}

	return *this;
}

PipelineBuilder& PipelineBuilder::set_render_primitive(RenderPrimitive prim) {
	_primitive_type = prim;

	return *this;
}

std::pair<Shader, Pipeline> PipelineBuilder::build(RenderPass render_pass) {
	auto device = Renderer::get_device();

	Shader shader = device->shader_create_from_bytecode(_shader_stages).value();

	// Create GraphicsPipelineCreateInfo
	GraphicsPipelineCreateInfo pipeline_info{
		.shader = shader,
		.primitive = _primitive_type,
		.vertex_input_state = _vertex_input,
		.rasterization_state = _rasterization,
		.multisample_state = _multisample,
		.depth_stencil_state = _depth_stencil_state,
		.color_blend_state = _color_blend_state,
		.dynamic_state = 0,
		.render_pass = render_pass,
		.rendering_info = _rendering_state,
	};

	Pipeline pipeline = device->graphics_pipeline_create(pipeline_info).value();

	return std::make_pair(shader, pipeline);
}

} //namespace gl