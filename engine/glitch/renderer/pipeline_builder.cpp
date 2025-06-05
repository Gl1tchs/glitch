#include "glitch/renderer/pipeline_builder.h"

#include "glitch/renderer/render_backend.h"
#include "glitch/renderer/render_device.h"

PipelineBuilder::PipelineBuilder() {
	vertex_input = {};
	rasterization = {};
	multisample = {};
	depth_stencil_state = {};
	color_blend_state = PipelineColorBlendState::create_disabled();
	rendering_state = {};
	rendering_state.color_attachments.push_back(
			RenderDevice::get_draw_image_format());
	rendering_state.depth_attachment = RenderDevice::get_depth_image_format();
}

PipelineBuilder& PipelineBuilder::add_shader_stage(
		ShaderStage p_stage, const std::vector<uint32_t>& p_spirv_data) {
	SpirvData shader_data = {};
	shader_data.stage = p_stage;
	shader_data.byte_code = p_spirv_data;

	shader_stages.push_back(shader_data);

	return *this;
}

PipelineBuilder& PipelineBuilder::with_depth_test(CompareOperator p_op) {
	depth_stencil_state.depth_compare_operator = COMPARE_OP_LESS;
	depth_stencil_state.enable_depth_test = true;
	depth_stencil_state.enable_depth_write = true;
	depth_stencil_state.enable_depth_range = true;

	return *this;
}

PipelineBuilder& PipelineBuilder::with_blend() {
	color_blend_state = PipelineColorBlendState::create_blend();

	return *this;
}

std::pair<Shader, Pipeline> PipelineBuilder::build() {
	Ref<RenderBackend> backend = RenderDevice::get_backend();

	Shader shader = backend->shader_create_from_bytecode(shader_stages);

	Pipeline pipeline =
			backend->render_pipeline_create(shader, RENDER_PRIMITIVE_TRIANGLES,
					vertex_input, rasterization, multisample,
					depth_stencil_state, color_blend_state, 0, rendering_state);

	return std::make_pair(shader, pipeline);
}