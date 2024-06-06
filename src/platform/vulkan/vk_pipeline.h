#pragma once

#include "core/templates/bit_field.h"

#include "renderer/types.h"

namespace vk {

Pipeline render_pipeline_create(Context p_context, Shader p_shader,
		RenderPrimitive p_render_primitive,
		PipelineRasterizationState p_rasterization_state,
		PipelineMultisampleState p_multisample_state,
		PipelineDepthStencilState p_depth_stencil_state,
		PipelineColorBlendState p_blend_state,
		BitField<PipelineDynamicStateFlags> p_dynamic_state,
		RenderingState p_rendering_state);

Pipeline compute_pipeline_create(Context p_context, Shader p_shader);

void pipeline_free(Context p_context, Pipeline p_pipeline);

} //namespace vk
