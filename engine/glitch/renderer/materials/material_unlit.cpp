#include "glitch/renderer/materials/material_unlit.h"

#include "glitch/renderer/render_backend.h"
#include "glitch/renderer/renderer.h"
#include "glitch/renderer/shader_library.h"
#include "glitch/renderer/types.h"

MaterialDefinition get_mat_unlit_definition() {
	Ref<RenderBackend> backend = Renderer::get_backend();

	SpirvData vertex_data = {};
	vertex_data.stage = SHADER_STAGE_VERTEX_BIT;
	vertex_data.byte_code = ShaderLibrary::get_bundled_spirv("mesh.vert.spv");

	SpirvData fragment_data = {};
	fragment_data.stage = SHADER_STAGE_FRAGMENT_BIT;
	fragment_data.byte_code =
			ShaderLibrary::get_bundled_spirv("unlit.frag.spv");

	std::vector<SpirvData> shader_stages = {
		vertex_data,
		fragment_data,
	};

	Shader shader = backend->shader_create_from_bytecode(shader_stages);

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

	Pipeline pipeline =
			backend->render_pipeline_create(shader, RENDER_PRIMITIVE_TRIANGLES,
					vertex_input, rasterization, multisample,
					depth_stencil_state, color_blend_state, 0, rendering_state);

	MaterialDefinition definition;
	definition.shader = shader;
	definition.pipeline = pipeline;
	definition.uniforms = {
		{ "base_color", 0, ShaderUniformVariableType::VEC4 },
		{ "metallic", 0, ShaderUniformVariableType::FLOAT },
		{ "roughness", 0, ShaderUniformVariableType::FLOAT },
		{ "u_albedo_texture", 1, ShaderUniformVariableType::TEXTURE },
		{ "u_normal_texture", 2, ShaderUniformVariableType::TEXTURE },
	};

	return definition;
}
