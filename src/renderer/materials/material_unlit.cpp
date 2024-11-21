#include "glitch/renderer/materials/material_unlit.h"

#include "glitch/renderer/render_backend.h"
#include "glitch/renderer/renderer.h"
#include "glitch/renderer/types.h"

MaterialUnlit::MaterialUnlit() {
	Ref<RenderBackend> backend = Renderer::get_backend();

	SpirvData vertex_data = {};
	vertex_data.stage = SHADER_STAGE_VERTEX_BIT;
	vertex_data.byte_code = get_bundled_spirv_data("mesh.vert.spv");

	SpirvData fragment_data = {};
	fragment_data.stage = SHADER_STAGE_FRAGMENT_BIT;
	fragment_data.byte_code = get_bundled_spirv_data("unlit.frag.spv");

	std::vector<SpirvData> shader_stages = {
		vertex_data,
		fragment_data,
	};

	shader = backend->shader_create_from_bytecode(shader_stages);

	PipelineVertexInputState vertex_input = {};
	PipelineRasterizationState rasterization = {};
	PipelineMultisampleState multisample = {};

	PipelineDepthStencilState depth_stencil_state = {};
	depth_stencil_state.depth_compare_operator = COMPARE_OP_LESS;
	depth_stencil_state.enable_depth_test = true;
	depth_stencil_state.enable_depth_write = true;
	depth_stencil_state.enable_depth_range = true;

	PipelineColorBlendState color_blend_state =
			PipelineColorBlendState::create_disabled();

	RenderingState rendering_state = {};
	rendering_state.color_attachments.push_back(
			Renderer::get_draw_image_format());
	rendering_state.depth_attachment = Renderer::get_depth_image_format();

	pipeline =
			backend->render_pipeline_create(shader, RENDER_PRIMITIVE_TRIANGLES,
					vertex_input, rasterization, multisample,
					depth_stencil_state, color_blend_state, 0, rendering_state);
}
