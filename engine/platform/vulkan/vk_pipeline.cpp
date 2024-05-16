#include "platform/vulkan/vk_pipeline.h"
#include "platform/vulkan/vk_init.h"

#include "shader_bundle.gen.h"

#include <vulkan/vulkan_core.h>

VulkanPipelineLayout::VulkanPipelineLayout(
		VkDevice device, const VulkanPipelineLayoutCreateInfo* info) :
		device(device) {
	VkPipelineLayoutCreateInfo pipeline_layout_info =
			vkinit::pipeline_layout_create_info();
	pipeline_layout_info.pushConstantRangeCount = info->push_constants.size();
	pipeline_layout_info.pPushConstantRanges = info->push_constants.data();
	pipeline_layout_info.setLayoutCount = info->descriptor_sets.size();
	pipeline_layout_info.pSetLayouts = info->descriptor_sets.data();

	VK_CHECK(vkCreatePipelineLayout(
			device, &pipeline_layout_info, nullptr, &layout));
}

VulkanPipelineLayout::~VulkanPipelineLayout() {
	vkDestroyPipelineLayout(device, layout, nullptr);
}

VulkanPipelineCreateInfo::VulkanPipelineCreateInfo() { reset(); }

void VulkanPipelineCreateInfo::reset() {
	input_assembly = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
	};

	rasterizer = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO
	};

	color_blend_attachment = {};

	multisampling = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
	};

	depth_stencil = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
	};

	render_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
	};

	shader_stages.clear();
}

void VulkanPipelineCreateInfo::set_shaders(
		VkShaderModule vertex_shader, VkShaderModule fragment_shader) {
	shader_stages.clear();

	shader_stages.push_back(vkinit::pipeline_shader_stage_create_info(
			VK_SHADER_STAGE_VERTEX_BIT, vertex_shader));

	shader_stages.push_back(vkinit::pipeline_shader_stage_create_info(
			VK_SHADER_STAGE_FRAGMENT_BIT, fragment_shader));
}

void VulkanPipelineCreateInfo::set_input_topology(
		VkPrimitiveTopology topology) {
	input_assembly.topology = topology;
	// TODO
	input_assembly.primitiveRestartEnable = false;
}

void VulkanPipelineCreateInfo::set_polygon_mode(VkPolygonMode mode) {
	rasterizer.polygonMode = mode;
	rasterizer.lineWidth = 1.0f;
}

void VulkanPipelineCreateInfo::set_cull_mode(
		VkCullModeFlags cull_mode, VkFrontFace front_face) {
	rasterizer.cullMode = cull_mode;
	rasterizer.frontFace = front_face;
}

void VulkanPipelineCreateInfo::set_multisampling_none() {
	multisampling.sampleShadingEnable = VK_FALSE;
	// multisampling defaulted to no multisampling (1 sample per pixel)
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = nullptr;
	// no alpha to coverage either
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;
}

void VulkanPipelineCreateInfo::enable_blending(
		VulkanBlendingMode blending_mode) {
	color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
			VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT;
	color_blend_attachment.blendEnable = VK_TRUE;
	color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
	color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
	color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

	switch (blending_mode) {
		case VulkanBlendingMode::ADDITIVE: {
			color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
			break;
		}
		case VulkanBlendingMode::ALPHA_BLEND: {
			color_blend_attachment.srcColorBlendFactor =
					VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
			break;
		}
	}
}

void VulkanPipelineCreateInfo::disable_blending() {
	// default write mask
	color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
			VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT;
	// no blending
	color_blend_attachment.blendEnable = VK_FALSE;
}

void VulkanPipelineCreateInfo::set_color_attachment_format(VkFormat format) {
	color_attachment_format = format;

	// connect the format to the render_info  structure
	render_info.colorAttachmentCount = 1;
	render_info.pColorAttachmentFormats = &color_attachment_format;
}

void VulkanPipelineCreateInfo::set_depth_format(VkFormat format) {
	render_info.depthAttachmentFormat = format;
}

void VulkanPipelineCreateInfo::enable_depthtest(
		bool depth_write_enable, VkCompareOp op) {
	depth_stencil.depthTestEnable = VK_TRUE;
	depth_stencil.depthWriteEnable = depth_write_enable;
	depth_stencil.depthCompareOp = op;
	depth_stencil.depthBoundsTestEnable = VK_FALSE;
	// TODO
	depth_stencil.stencilTestEnable = VK_FALSE;
	depth_stencil.front = {};
	depth_stencil.back = {};
	depth_stencil.minDepthBounds = 0.0f;
	depth_stencil.maxDepthBounds = 1.0f;
}

void VulkanPipelineCreateInfo::disable_depthtest() {
	depth_stencil.depthTestEnable = VK_FALSE;
	depth_stencil.depthWriteEnable = VK_FALSE;
	depth_stencil.depthCompareOp = VK_COMPARE_OP_NEVER;
	depth_stencil.depthBoundsTestEnable = VK_FALSE;
	depth_stencil.stencilTestEnable = VK_FALSE;
	depth_stencil.front = {};
	depth_stencil.back = {};
	depth_stencil.minDepthBounds = 0.0f;
	depth_stencil.maxDepthBounds = 1.0f;
}

VulkanPipeline::VulkanPipeline(VkDevice device,
		const VulkanPipelineCreateInfo* info,
		const VulkanPipelineLayout* layout) :
		device(device), pipeline(VK_NULL_HANDLE) {
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
		.attachmentCount = 1,
		.pAttachments = &info->color_blend_attachment,
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
		.pNext = &info->render_info,
		.stageCount = (uint32_t)info->shader_stages.size(),
		.pStages = info->shader_stages.data(),
		.pVertexInputState = &vertex_input_info,
		.pInputAssemblyState = &info->input_assembly,
		.pViewportState = &viewport_state,
		.pRasterizationState = &info->rasterizer,
		.pMultisampleState = &info->multisampling,
		.pDepthStencilState = &info->depth_stencil,
		.pColorBlendState = &color_blending,
		.pDynamicState = &dynamic_info,
		.layout = layout->layout,
	};

	// its easy to error out on create graphics pipeline, so we handle
	// it a bit better than the common VK_CHECK case
	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_info,
				nullptr, &pipeline) != VK_SUCCESS) {
		GL_LOG_ERROR("Failed to create pipeline!");
	}
}

VulkanPipeline::~VulkanPipeline() {
	vkDestroyPipeline(device, pipeline, nullptr);
}

void VulkanPipeline::bind(VkCommandBuffer cmd) {
	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}

bool vk_load_shader_module(VkDevice device, const char* file_path,
		VkShaderModule* out_shader_module) {
	BundleFileData shader_data{};
	bool shader_found = false;

	for (int i = 0; i < BUNDLE_FILE_COUNT; i++) {
		BundleFileData data = BUNDLE_FILES[i];
		if (strcmp(data.path, file_path) == 0) {
			shader_data = data;
			shader_found = true;
			break;
		}
	}

	if (!shader_found) {
		return false;
	}

	// create a new shader module, using the buffer we loaded
	VkShaderModuleCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_info.pNext = nullptr;

	// codeSize has to be in bytes, so multiply the ints in the buffer by size
	// of int to know the real size of the buffer
	create_info.codeSize = shader_data.size;
	create_info.pCode = (uint32_t*)&BUNDLE_DATA[shader_data.start_idx];

	// check that the creation goes well.
	VkShaderModule shader_module;
	if (vkCreateShaderModule(device, &create_info, nullptr, &shader_module) !=
			VK_SUCCESS) {
		return false;
	}

	*out_shader_module = shader_module;
	return true;
}
