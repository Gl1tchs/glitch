/**
 * @file pipeline_builder.h
 *
 */

#pragma once

#include "glitch/renderer/types.h"

class PipelineBuilder {
public:
	PipelineBuilder();

	PipelineBuilder& add_shader_stage(
			ShaderStage p_stage, const std::vector<uint32_t>& p_spirv_data);

	PipelineBuilder& with_depth_test(
			CompareOperator p_op = COMPARE_OP_LESS, bool p_depth_write = true);

	PipelineBuilder& with_blend();

	/**
	 * @param p_render_pass Optional render pass to build pipeline with. Default
	 * will assume dynamic rendering.
	 */
	std::pair<Shader, Pipeline> build(RenderPass p_render_pass = nullptr);

private:
	std::vector<SpirvData> shader_stages;

	PipelineVertexInputState vertex_input;
	PipelineRasterizationState rasterization;
	PipelineMultisampleState multisample;
	PipelineDepthStencilState depth_stencil_state;
	PipelineColorBlendState color_blend_state;
	RenderingState rendering_state;
};
