/**
 * @file pipeline_builder.h
 *
 */

#pragma once

#include <glgpu/glgpu.h>

#include <optional>
#include <vector>

namespace gl {

class PipelineBuilder {
public:
	PipelineBuilder();

	PipelineBuilder& add_color_attachment(DataFormat format);

	PipelineBuilder& set_depth_attachment(std::optional<DataFormat> format);

	PipelineBuilder& add_shader_stage(
			ShaderStageFlags stage, const std::vector<uint32_t>& spirv_data);

	PipelineBuilder& with_depth_test(
			CompareOperator op = CompareOperator::LESS, bool depth_write = true);

	PipelineBuilder& with_blend();

	PipelineBuilder& with_multisample(uint32_t samples, bool enable_sample_shading = false);

	PipelineBuilder& set_render_primitive(RenderPrimitive prim);

	/**
	 * @param p_render_pass Optional render pass to build pipeline with. Default
	 * will assume dynamic rendering.
	 */
	std::pair<Shader, Pipeline> build(RenderPass render_pass = nullptr);

private:
	std::vector<SpirvEntry> _shader_stages;

	RenderPrimitive _primitive_type;
	PipelineVertexInputState _vertex_input;
	PipelineRasterizationState _rasterization;
	PipelineMultisampleState _multisample;
	PipelineDepthStencilState _depth_stencil_state;
	PipelineColorBlendState _color_blend_state;
	PipelineRenderingState _rendering_state;
};

} //namespace gl