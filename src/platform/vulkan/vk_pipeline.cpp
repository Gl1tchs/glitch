#include "platform/vulkan/vk_pipeline.h"
#include "platform/vulkan/vk_init.h"

#include <vulkan/vulkan_core.h>

VulkanPipelineLayout VulkanPipelineLayout::create(
		VkDevice device, const VulkanPipelineLayoutCreateInfo* info) {
	VkPipelineLayoutCreateInfo pipeline_layout_info =
			vkinit::pipeline_layout_create_info();
	pipeline_layout_info.pushConstantRangeCount = info->push_constant_count;
	pipeline_layout_info.pPushConstantRanges = info->push_constants;
	pipeline_layout_info.setLayoutCount = info->descriptor_set_count;
	pipeline_layout_info.pSetLayouts = info->descriptor_sets;

	VulkanPipelineLayout layout{ VK_NULL_HANDLE };
	VK_CHECK(vkCreatePipelineLayout(
			device, &pipeline_layout_info, nullptr, &layout.layout));

	return layout;
}

void VulkanPipelineLayout::destroy(
		VkDevice device, const VulkanPipelineLayout* layout) {
	vkDestroyPipelineLayout(device, layout->layout, nullptr);
}

static VkPipelineInputAssemblyStateCreateInfo get_input_assembly_info(
		const VulkanPipelineCreateInfo* info) {
	VkPipelineInputAssemblyStateCreateInfo input_assembly = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = info->topology,
		// TODO
		.primitiveRestartEnable = false,
	};
	return input_assembly;
}

static VkPipelineRasterizationStateCreateInfo get_rasterizer_info(
		const VulkanPipelineCreateInfo* info) {
	VkPipelineRasterizationStateCreateInfo rasterizer = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.polygonMode = info->polygon_mode,
		.cullMode = info->cull_mode,
		.frontFace = info->front_face,
		.lineWidth = 1.0f,
	};
	return rasterizer;
}

static VkPipelineMultisampleStateCreateInfo get_multisampling_info(
		const VulkanPipelineCreateInfo* info) {
	VkPipelineMultisampleStateCreateInfo multisampling = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
		.sampleShadingEnable = VK_FALSE,
		// multisampling defaulted to no multisampling (1 sample per pixel)
		.minSampleShading = 1.0f,
		.pSampleMask = nullptr,
		// no alpha to coverage either
		.alphaToCoverageEnable = VK_FALSE,
		.alphaToOneEnable = VK_FALSE,
	};
	return multisampling;
}

static std::vector<VkPipelineColorBlendAttachmentState>
get_color_blend_attachment_states(const VulkanPipelineCreateInfo* info) {
	std::vector<VkPipelineColorBlendAttachmentState> color_blend_attachments(
			info->color_attachments.size());
	for (auto& color_blend_attachment : color_blend_attachments) {
		color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
				VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
				VK_COLOR_COMPONENT_A_BIT;

		switch (info->blending_mode) {
			case VulkanBlendingMode::ADDITIVE: {
				color_blend_attachment.blendEnable = VK_TRUE;
				color_blend_attachment.dstColorBlendFactor =
						VK_BLEND_FACTOR_DST_ALPHA;
				color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
				color_blend_attachment.srcAlphaBlendFactor =
						VK_BLEND_FACTOR_ONE;
				color_blend_attachment.dstAlphaBlendFactor =
						VK_BLEND_FACTOR_ZERO;
				color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
				color_blend_attachment.srcColorBlendFactor =
						VK_BLEND_FACTOR_ONE;
				break;
			}
			case VulkanBlendingMode::ALPHA_BLEND: {
				color_blend_attachment.blendEnable = VK_TRUE;
				color_blend_attachment.dstColorBlendFactor =
						VK_BLEND_FACTOR_DST_ALPHA;
				color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
				color_blend_attachment.srcAlphaBlendFactor =
						VK_BLEND_FACTOR_ONE;
				color_blend_attachment.dstAlphaBlendFactor =
						VK_BLEND_FACTOR_ZERO;
				color_blend_attachment.srcColorBlendFactor =
						VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
				break;
			}
			case VulkanBlendingMode::NONE: {
				color_blend_attachment.blendEnable = VK_FALSE;
				break;
			}
		}
	}

	return color_blend_attachments;
}

static VkPipelineDepthStencilStateCreateInfo get_depth_stencil_info(
		const VulkanPipelineCreateInfo* info) {
	VkPipelineDepthStencilStateCreateInfo depth_stencil = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		// TODO
		.stencilTestEnable = VK_FALSE,
		.front = {},
		.back = {},
		.minDepthBounds = 0.0f,
		.maxDepthBounds = 1.0f,
	};

	if (info->enable_depth_test) {
		depth_stencil.depthTestEnable = VK_TRUE;
		depth_stencil.depthWriteEnable = info->depth_write_enable;
		depth_stencil.depthCompareOp = info->depth_op;
		depth_stencil.depthBoundsTestEnable = VK_FALSE;
	} else {
		depth_stencil.depthTestEnable = VK_FALSE;
		depth_stencil.depthWriteEnable = VK_FALSE;
		depth_stencil.depthCompareOp = VK_COMPARE_OP_NEVER;
		depth_stencil.depthBoundsTestEnable = VK_FALSE;
	}

	return depth_stencil;
}

static VkPipelineRenderingCreateInfo get_render_info(
		const VulkanPipelineCreateInfo* info) {
	VkPipelineRenderingCreateInfo render_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
		// connect the format to the render_info  structure
		.colorAttachmentCount = (uint32_t)info->color_attachments.size(),
		.pColorAttachmentFormats = info->color_attachments.data(),
		.depthAttachmentFormat = info->depth_attachment,
	};

	return render_info;
}

VulkanPipeline VulkanPipeline::create(VkDevice device,
		const VulkanPipelineCreateInfo* info,
		const VulkanPipelineLayout* layout) {
	std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
	shader_stages.push_back(vkinit::pipeline_shader_stage_create_info(
			VK_SHADER_STAGE_VERTEX_BIT, info->vertex_shader->shader));
	shader_stages.push_back(vkinit::pipeline_shader_stage_create_info(
			VK_SHADER_STAGE_FRAGMENT_BIT, info->fragment_shader->shader));

	VkPipelineInputAssemblyStateCreateInfo input_assembly =
			get_input_assembly_info(info);

	VkPipelineRasterizationStateCreateInfo rasterizer =
			get_rasterizer_info(info);

	VkPipelineMultisampleStateCreateInfo multisampling =
			get_multisampling_info(info);

	std::vector<VkPipelineColorBlendAttachmentState> color_blend_states =
			get_color_blend_attachment_states(info);

	VkPipelineDepthStencilStateCreateInfo depth_stencil =
			get_depth_stencil_info(info);

	VkPipelineRenderingCreateInfo render_info = get_render_info(info);

	// make viewport state from our stored viewport and scissor.
	// at the moment we wont support multiple viewports or scissors
	VkPipelineViewportStateCreateInfo viewport_state = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.pNext = nullptr,
		.viewportCount = 1,
		.scissorCount = 1,
	};

	// setup dummy color blending. We arent using transparent objects yet
	// the blending is just "no blend", but we do write to the color attachment
	VkPipelineColorBlendStateCreateInfo color_blending = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.pNext = nullptr,
		.logicOpEnable = VK_FALSE,
		.logicOp = VK_LOGIC_OP_COPY,
		.attachmentCount = static_cast<uint32_t>(color_blend_states.size()),
		.pAttachments = color_blend_states.data(),
	};

	// completely clear VertexInputStateCreateInfo, as we have no need for it
	VkPipelineVertexInputStateCreateInfo vertex_input_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
	};

	// create dynamic state
	VkDynamicState state[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
	};

	VkPipelineDynamicStateCreateInfo dynamic_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = 2,
		.pDynamicStates = &state[0],
	};

	// build the actual pipeline
	// we now use all of the info structs we have been writing into into this
	// one to create the pipeline
	VkGraphicsPipelineCreateInfo pipeline_info = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		// connect the renderInfo to the pNext extension mechanism
		.pNext = &render_info,
		.stageCount = (uint32_t)shader_stages.size(),
		.pStages = shader_stages.data(),
		.pVertexInputState = &vertex_input_info,
		.pInputAssemblyState = &input_assembly,
		.pViewportState = &viewport_state,
		.pRasterizationState = &rasterizer,
		.pMultisampleState = &multisampling,
		.pDepthStencilState = &depth_stencil,
		.pColorBlendState = &color_blending,
		.pDynamicState = &dynamic_info,
		.layout = layout->layout,
	};

	VulkanPipeline pipeline{ VK_NULL_HANDLE };

	// its easy to error out on create graphics pipeline, so we handle
	// it a bit better than the common VK_CHECK case
	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_info,
				nullptr, &pipeline.pipeline) != VK_SUCCESS) {
		GL_LOG_ERROR("Failed to create pipeline!");
	}

	return pipeline;
}

void VulkanPipeline::destroy(VkDevice device, VulkanPipeline& pipeline) {
	vkDestroyPipeline(device, pipeline.pipeline, nullptr);
}
