#include "glitch/platform/vulkan/vk_backend.h"

#include "glitch/renderer/types.h"

#include <vulkan/vulkan_core.h>

namespace gl {

static VkCullModeFlagBits _gl_to_vk_cull_mode(PolygonCullMode cull_mode) {
	switch (cull_mode) {
		case PolygonCullMode::DISABLED:
			return VK_CULL_MODE_NONE;
		case PolygonCullMode::FRONT:
			return VK_CULL_MODE_FRONT_BIT;
		case PolygonCullMode::BACK:
			return VK_CULL_MODE_BACK_BIT;
		default:
			return VK_CULL_MODE_FLAG_BITS_MAX_ENUM;
	}
}

constexpr uint32_t PIPELINE_CACHE_MAGIC_NUMBER = 0xbba786cf;

struct PipelineCacheHeader {
	uint32_t magic_number; // PIPELINE_MAGIC_NUMBER
	size_t data_size; // size of the data
	uint32_t vendor_id; // VkPhysicalDeviceProperties::vendorID
	uint32_t device_id; // VkPhysicalDeviceProperties::deviceID
	uint32_t driver_version; // VkPhysicalDeviceProperties::driverVersion
	uint8_t uuid[VK_UUID_SIZE]; // VkPhysicalDeviceProperties::pipelineCacheUUID
};

static VkPipelineCache _load_pipeline_cache(VkDevice p_device, const fs::path& p_path,
		const VkPhysicalDeviceProperties& p_device_props) {
	VkPipelineCacheCreateInfo cache_create_info = {};
	cache_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

	// if cache already exists on disk try to load it
	const bool cache_exists = fs::exists(p_path);
	if (cache_exists) {
		std::ifstream file(p_path, std::ios::binary);
		if (!file) {
			GL_LOG_ERROR("[VULKAN] [_load_pipeline_cache] Unable to parse pipeline cache at '{}'",
					p_path.string());
			return {};
		}

		file.seekg(0, std::ios::end);

		const size_t cache_size = file.tellg();

		file.seekg(0, std::ios::beg);

		std::vector<char> cache(cache_size);
		file.read(cache.data(), cache_size);

		file.close();

		const char* cache_data = cache.data() + sizeof(PipelineCacheHeader);
		int64_t cache_data_size = cache.size() - sizeof(PipelineCacheHeader);

		if (!cache.empty() || cache_data_size > 0) {
			// verify integrity of the data
			bool valid = [&]() -> bool {
				const PipelineCacheHeader* header =
						reinterpret_cast<const PipelineCacheHeader*>(cache.data());

				if (header->magic_number != PIPELINE_CACHE_MAGIC_NUMBER) {
					return false;
				}
				if (header->data_size != cache_data_size) {
					return false;
				}
				if (header->vendor_id != p_device_props.vendorID) {
					return false;
				}
				if (header->device_id != p_device_props.deviceID) {
					return false;
				}
				if (header->driver_version != p_device_props.driverVersion) {
					return false;
				}
				for (uint32_t i = 0; i < VK_UUID_SIZE; i++) {
					if (header->uuid[i] != p_device_props.pipelineCacheUUID[i]) {
						return false;
					}
				}

				return true;
			}();

			cache_create_info.initialDataSize = valid ? cache_data_size : 0;
			cache_create_info.pInitialData = cache_data;
		}
	}

	VkPipelineCache vk_pipeline_cache = VK_NULL_HANDLE;
	VK_CHECK(vkCreatePipelineCache(p_device, &cache_create_info, nullptr, &vk_pipeline_cache));

	return vk_pipeline_cache;
}

static VkPipelineVertexInputStateCreateInfo _get_vertex_input_state_info(
		VulkanRenderBackend* backend, Shader p_shader,
		PipelineVertexInputState p_vertex_input_state) {
	VkVertexInputBindingDescription vertex_binding = {};
	std::vector<VkVertexInputAttributeDescription> vertex_attributes;

	// only populate if there is any vertex input configured
	if (p_vertex_input_state.stride != 0) {
		vertex_binding.binding = 0;
		vertex_binding.stride = p_vertex_input_state.stride;
		vertex_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		for (const auto& input : backend->shader_get_vertex_inputs(p_shader)) {
			VkVertexInputAttributeDescription attribute = {};
			attribute.binding = 0;
			attribute.format = static_cast<VkFormat>(input.format);
			attribute.location = input.location;
			attribute.offset = 0;

			vertex_attributes.push_back(attribute);
		}

		// attributes should be sorted by location then assign the offsets
		std::sort(vertex_attributes.begin(), vertex_attributes.end(),
				[](const auto& lhs, const auto& rhs) { return lhs.location < rhs.location; });

		uint32_t offset = 0;
		for (auto& attr : vertex_attributes) {
			size_t data_size = get_data_format_size(static_cast<DataFormat>(attr.format));
			GL_ASSERT(data_size != 0, "Unsupported data type to calculate size.");

			attr.offset = offset;
			offset += data_size;
		}
	}

	VkPipelineVertexInputStateCreateInfo vertex_info = {};
	vertex_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_info.vertexBindingDescriptionCount = p_vertex_input_state.stride != 0 ? 1 : 0;
	vertex_info.pVertexBindingDescriptions = &vertex_binding;
	vertex_info.vertexAttributeDescriptionCount = vertex_attributes.size();
	vertex_info.pVertexAttributeDescriptions = vertex_attributes.data();

	return vertex_info;
}

static VkPipelineInputAssemblyStateCreateInfo _get_input_assembly_state_info(
		RenderPrimitive p_render_primitive) {
	VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
	input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly.topology = static_cast<VkPrimitiveTopology>(p_render_primitive);
	input_assembly.primitiveRestartEnable = false;

	return input_assembly;
}

static VkPipelineRasterizationStateCreateInfo _get_rasterization_state_info(
		PipelineRasterizationState p_rasterization_state) {
	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = p_rasterization_state.enable_depth_clamp;
	rasterizer.rasterizerDiscardEnable = p_rasterization_state.discard_primitives;
	rasterizer.polygonMode =
			p_rasterization_state.wireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
	rasterizer.cullMode = _gl_to_vk_cull_mode(p_rasterization_state.cull_mode);
	rasterizer.frontFace = p_rasterization_state.front_face == PolygonFrontFace::CLOCKWISE
			? VK_FRONT_FACE_CLOCKWISE
			: VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = p_rasterization_state.depth_bias_enabled;
	rasterizer.depthBiasClamp = p_rasterization_state.depth_bias_clamp;
	rasterizer.depthBiasSlopeFactor = p_rasterization_state.depth_bias_slope_factor;
	rasterizer.lineWidth = p_rasterization_state.line_width;

	return rasterizer;
}

static VkPipelineMultisampleStateCreateInfo _get_multisampling_state_info(
		PipelineMultisampleState p_multisample_state) {
	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.rasterizationSamples =
			static_cast<VkSampleCountFlagBits>(p_multisample_state.sample_count);
	multisampling.sampleShadingEnable = p_multisample_state.enable_sample_shading;
	multisampling.minSampleShading = p_multisample_state.min_sample_shading;
	multisampling.pSampleMask = (VkSampleMask*)p_multisample_state.sample_mask.data();
	multisampling.alphaToCoverageEnable = p_multisample_state.enable_alpha_to_coverage;
	multisampling.alphaToOneEnable = p_multisample_state.enable_alpha_to_one;

	return multisampling;
}

static VkPipelineDepthStencilStateCreateInfo _get_depth_stencil_state_info(
		PipelineDepthStencilState p_depth_stencil_state) {
	VkPipelineDepthStencilStateCreateInfo depth_stencil = {};
	depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depth_stencil.depthTestEnable = p_depth_stencil_state.enable_depth_test;
	depth_stencil.depthWriteEnable = p_depth_stencil_state.enable_depth_write;
	depth_stencil.depthCompareOp =
			static_cast<VkCompareOp>(p_depth_stencil_state.depth_compare_operator);
	depth_stencil.depthBoundsTestEnable = p_depth_stencil_state.enable_depth_range;
	depth_stencil.minDepthBounds = p_depth_stencil_state.depth_range_min;
	depth_stencil.maxDepthBounds = p_depth_stencil_state.depth_range_max;
	depth_stencil.stencilTestEnable = p_depth_stencil_state.enable_stencil;

	VkStencilOpState front_stencil_state;
	front_stencil_state.failOp = static_cast<VkStencilOp>(p_depth_stencil_state.front_op.fail);
	front_stencil_state.passOp = static_cast<VkStencilOp>(p_depth_stencil_state.front_op.pass);
	front_stencil_state.depthFailOp =
			static_cast<VkStencilOp>(p_depth_stencil_state.front_op.depth_fail);
	front_stencil_state.compareOp =
			static_cast<VkCompareOp>(p_depth_stencil_state.front_op.compare);
	front_stencil_state.compareMask = p_depth_stencil_state.front_op.compare_mask;
	front_stencil_state.writeMask = p_depth_stencil_state.front_op.write_mask;
	front_stencil_state.reference = p_depth_stencil_state.front_op.reference;

	depth_stencil.front = front_stencil_state;

	VkStencilOpState back_stencil_state;
	back_stencil_state.failOp = static_cast<VkStencilOp>(p_depth_stencil_state.back_op.fail);
	back_stencil_state.passOp = static_cast<VkStencilOp>(p_depth_stencil_state.back_op.pass);
	back_stencil_state.depthFailOp =
			static_cast<VkStencilOp>(p_depth_stencil_state.back_op.depth_fail);
	back_stencil_state.compareOp = static_cast<VkCompareOp>(p_depth_stencil_state.back_op.compare);
	back_stencil_state.compareMask = p_depth_stencil_state.back_op.compare_mask;
	back_stencil_state.writeMask = p_depth_stencil_state.back_op.write_mask;
	back_stencil_state.reference = p_depth_stencil_state.back_op.reference;

	depth_stencil.back = back_stencil_state;

	return depth_stencil;
}

static std::vector<VkPipelineColorBlendAttachmentState> _get_color_blend_attachments(
		PipelineColorBlendState p_blend_state) {
	std::vector<VkPipelineColorBlendAttachmentState> color_blend_attachments;
	for (const auto& attachment : p_blend_state.attachments) {
		VkPipelineColorBlendAttachmentState vk_attachment = {};
		vk_attachment.blendEnable = attachment.enable_blend;
		vk_attachment.srcColorBlendFactor =
				static_cast<VkBlendFactor>(attachment.src_color_blend_factor);
		vk_attachment.dstColorBlendFactor =
				static_cast<VkBlendFactor>(attachment.dst_color_blend_factor);
		vk_attachment.colorBlendOp = static_cast<VkBlendOp>(attachment.color_blend_op);
		vk_attachment.srcAlphaBlendFactor =
				static_cast<VkBlendFactor>(attachment.src_alpha_blend_factor);
		vk_attachment.dstAlphaBlendFactor =
				static_cast<VkBlendFactor>(attachment.dst_alpha_blend_factor);
		vk_attachment.alphaBlendOp = static_cast<VkBlendOp>(attachment.alpha_blend_op);

		if (attachment.write_r) {
			vk_attachment.colorWriteMask |= VK_COLOR_COMPONENT_R_BIT;
		}
		if (attachment.write_g) {
			vk_attachment.colorWriteMask |= VK_COLOR_COMPONENT_G_BIT;
		}
		if (attachment.write_b) {
			vk_attachment.colorWriteMask |= VK_COLOR_COMPONENT_B_BIT;
		}
		if (attachment.write_a) {
			vk_attachment.colorWriteMask |= VK_COLOR_COMPONENT_A_BIT;
		}

		color_blend_attachments.push_back(vk_attachment);
	}

	return color_blend_attachments;
}

static VkPipelineColorBlendStateCreateInfo _get_color_blend_state_info(
		PipelineColorBlendState p_blend_state,
		const std::vector<VkPipelineColorBlendAttachmentState>& p_attachments) {
	VkPipelineColorBlendStateCreateInfo color_blend = {};
	color_blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blend.logicOpEnable = p_blend_state.enable_logic_op;
	color_blend.logicOp = static_cast<VkLogicOp>(p_blend_state.logic_op);
	color_blend.attachmentCount = static_cast<uint32_t>(p_attachments.size());
	color_blend.pAttachments = p_attachments.data();

	color_blend.blendConstants[0] = p_blend_state.blend_constant.x;
	color_blend.blendConstants[1] = p_blend_state.blend_constant.y;
	color_blend.blendConstants[2] = p_blend_state.blend_constant.z;
	color_blend.blendConstants[3] = p_blend_state.blend_constant.w;

	return color_blend;
}

static std::vector<VkDynamicState> _get_dynamic_states(PipelineDynamicStateFlags p_dynamic_state) {
	std::vector<VkDynamicState> states = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
	};

	if (p_dynamic_state & DYNAMIC_STATE_LINE_WIDTH) {
		states.push_back(VK_DYNAMIC_STATE_LINE_WIDTH);
	}
	if (p_dynamic_state & DYNAMIC_STATE_DEPTH_BIAS) {
		states.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);
	}
	if (p_dynamic_state & DYNAMIC_STATE_BLEND_CONSTANTS) {
		states.push_back(VK_DYNAMIC_STATE_BLEND_CONSTANTS);
	}
	if (p_dynamic_state & DYNAMIC_STATE_DEPTH_BOUNDS) {
		states.push_back(VK_DYNAMIC_STATE_DEPTH_BOUNDS);
	}
	if (p_dynamic_state & DYNAMIC_STATE_STENCIL_COMPARE_MASK) {
		states.push_back(VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK);
	}
	if (p_dynamic_state & DYNAMIC_STATE_STENCIL_WRITE_MASK) {
		states.push_back(VK_DYNAMIC_STATE_STENCIL_WRITE_MASK);
	}
	if (p_dynamic_state & DYNAMIC_STATE_STENCIL_REFERENCE) {
		states.push_back(VK_DYNAMIC_STATE_STENCIL_REFERENCE);
	}

	return states;
}

static VkPipelineDynamicStateCreateInfo _get_dynamic_state_info(
		const std::vector<VkDynamicState>& p_states) {
	VkPipelineDynamicStateCreateInfo dynamic_state = {};
	dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_state.dynamicStateCount = static_cast<uint32_t>(p_states.size());
	dynamic_state.pDynamicStates = p_states.data();

	return dynamic_state;
}

constexpr static VkPipelineViewportStateCreateInfo _get_viewport_state() {
	VkPipelineViewportStateCreateInfo viewport_state = {};
	viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state.viewportCount = 1;
	viewport_state.scissorCount = 1;

	return viewport_state;
}

Pipeline VulkanRenderBackend::render_pipeline_create(Shader p_shader,
		RenderPrimitive p_render_primitive, PipelineVertexInputState p_vertex_input_state,
		PipelineRasterizationState p_rasterization_state,
		PipelineMultisampleState p_multisample_state,
		PipelineDepthStencilState p_depth_stencil_state, PipelineColorBlendState p_blend_state,
		PipelineDynamicStateFlags p_dynamic_state, RenderingState p_rendering_state) {
	// Vertex info
	const VkPipelineVertexInputStateCreateInfo vertex_info =
			_get_vertex_input_state_info(this, p_shader, p_vertex_input_state);

	// Input assembly state
	const VkPipelineInputAssemblyStateCreateInfo input_assembly =
			_get_input_assembly_state_info(p_render_primitive);

	// Rasterizer state
	const VkPipelineRasterizationStateCreateInfo rasterizer =
			_get_rasterization_state_info(p_rasterization_state);

	// Multisampling state
	const VkPipelineMultisampleStateCreateInfo multisampling =
			_get_multisampling_state_info(p_multisample_state);

	// Depth stencil state
	const VkPipelineDepthStencilStateCreateInfo depth_stencil =
			_get_depth_stencil_state_info(p_depth_stencil_state);

	// Color blend attachment
	const std::vector<VkPipelineColorBlendAttachmentState> color_blend_attachments =
			_get_color_blend_attachments(p_blend_state);
	const VkPipelineColorBlendStateCreateInfo color_blend =
			_get_color_blend_state_info(p_blend_state, color_blend_attachments);

	// Dynamic state
	const std::vector<VkDynamicState> dynamic_states = _get_dynamic_states(p_dynamic_state);

	const VkPipelineDynamicStateCreateInfo dynamic_state = _get_dynamic_state_info(dynamic_states);

	// Viewport state
	const VkPipelineViewportStateCreateInfo viewport_state = _get_viewport_state();

	// Rendering state
	std::vector<VkFormat> vk_color_attachments;
	for (size_t i = 0; i < p_rendering_state.color_attachments.size(); i++) {
		vk_color_attachments.push_back(
				static_cast<VkFormat>(p_rendering_state.color_attachments[i]));
	}

	VkPipelineRenderingCreateInfo rendering_info = {};
	rendering_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
	rendering_info.colorAttachmentCount = static_cast<uint32_t>(vk_color_attachments.size());
	rendering_info.pColorAttachmentFormats = vk_color_attachments.data();
	rendering_info.depthAttachmentFormat =
			static_cast<VkFormat>(p_rendering_state.depth_attachment);

	VulkanShader* shader = (VulkanShader*)p_shader;

	VkGraphicsPipelineCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	create_info.pNext = &rendering_info,
	create_info.stageCount = static_cast<uint32_t>(shader->stage_create_infos.size());
	create_info.pStages = shader->stage_create_infos.data();
	create_info.pVertexInputState = &vertex_info;
	create_info.pInputAssemblyState = &input_assembly;
	create_info.pViewportState = &viewport_state;
	create_info.pRasterizationState = &rasterizer;
	create_info.pMultisampleState = &multisampling;
	create_info.pDepthStencilState = &depth_stencil;
	create_info.pColorBlendState = &color_blend;
	create_info.pDynamicState = &dynamic_state;
	create_info.layout = shader->pipeline_layout;

	std::string cache_path = std::format(".glitch/cache/{}.cache", shader->shader_hash);

	VkPipelineCache vk_pipeline_cache =
			_load_pipeline_cache(device, cache_path, physical_device_properties);

	VkPipeline vk_pipeline = VK_NULL_HANDLE;
	VK_CHECK(vkCreateGraphicsPipelines(
			device, vk_pipeline_cache, 1, &create_info, nullptr, &vk_pipeline));

	VulkanPipeline* pipeline = VersatileResource::allocate<VulkanPipeline>(resources_allocator);
	pipeline->vk_pipeline = vk_pipeline;
	pipeline->vk_pipeline_cache = vk_pipeline_cache;
	pipeline->shader_hash = shader->shader_hash;

	return Pipeline(pipeline);
}

Pipeline VulkanRenderBackend::render_pipeline_create(Shader p_shader, RenderPass p_render_pass,
		RenderPrimitive p_render_primitive, PipelineVertexInputState p_vertex_input_state,
		PipelineRasterizationState p_rasterization_state,
		PipelineMultisampleState p_multisample_state,
		PipelineDepthStencilState p_depth_stencil_state, PipelineColorBlendState p_blend_state,
		PipelineDynamicStateFlags p_dynamic_state) {
	// Vertex info
	const VkPipelineVertexInputStateCreateInfo vertex_info =
			_get_vertex_input_state_info(this, p_shader, p_vertex_input_state);

	// Input assembly state
	const VkPipelineInputAssemblyStateCreateInfo input_assembly =
			_get_input_assembly_state_info(p_render_primitive);

	// Rasterizer state
	const VkPipelineRasterizationStateCreateInfo rasterizer =
			_get_rasterization_state_info(p_rasterization_state);

	// Multisampling state
	const VkPipelineMultisampleStateCreateInfo multisampling =
			_get_multisampling_state_info(p_multisample_state);

	// Depth stencil state
	const VkPipelineDepthStencilStateCreateInfo depth_stencil =
			_get_depth_stencil_state_info(p_depth_stencil_state);

	// Color blend attachment
	const std::vector<VkPipelineColorBlendAttachmentState> color_blend_attachments =
			_get_color_blend_attachments(p_blend_state);
	const VkPipelineColorBlendStateCreateInfo color_blend =
			_get_color_blend_state_info(p_blend_state, color_blend_attachments);

	// Dynamic state
	const std::vector<VkDynamicState> dynamic_states = _get_dynamic_states(p_dynamic_state);

	const VkPipelineDynamicStateCreateInfo dynamic_state = _get_dynamic_state_info(dynamic_states);

	// Viewport state
	const VkPipelineViewportStateCreateInfo viewport_state = _get_viewport_state();

	VulkanShader* shader = (VulkanShader*)p_shader;
	VulkanRenderPass* render_pass = (VulkanRenderPass*)p_render_pass;

	VkGraphicsPipelineCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	create_info.renderPass = render_pass->vk_render_pass;
	create_info.stageCount = static_cast<uint32_t>(shader->stage_create_infos.size());
	create_info.pStages = shader->stage_create_infos.data();
	create_info.pVertexInputState = &vertex_info;
	create_info.pInputAssemblyState = &input_assembly;
	create_info.pViewportState = &viewport_state;
	create_info.pRasterizationState = &rasterizer;
	create_info.pMultisampleState = &multisampling;
	create_info.pDepthStencilState = &depth_stencil;
	create_info.pColorBlendState = &color_blend;
	create_info.pDynamicState = &dynamic_state;
	create_info.layout = shader->pipeline_layout;

	std::string cache_path = std::format(".glitch/cache/{}.cache", shader->shader_hash);

	VkPipelineCache vk_pipeline_cache =
			_load_pipeline_cache(device, cache_path, physical_device_properties);

	VkPipeline vk_pipeline = VK_NULL_HANDLE;
	VK_CHECK(vkCreateGraphicsPipelines(
			device, vk_pipeline_cache, 1, &create_info, nullptr, &vk_pipeline));

	VulkanPipeline* pipeline = VersatileResource::allocate<VulkanPipeline>(resources_allocator);
	pipeline->vk_pipeline = vk_pipeline;
	pipeline->vk_pipeline_cache = vk_pipeline_cache;
	pipeline->shader_hash = shader->shader_hash;

	return Pipeline(pipeline);
}

Pipeline VulkanRenderBackend::compute_pipeline_create(Shader p_shader) {
	VulkanShader* shader = (VulkanShader*)p_shader;

	VkComputePipelineCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	create_info.stage = shader->stage_create_infos[0];
	create_info.layout = shader->pipeline_layout;

	std::string cache_path = std::format(".glitch/cache/{}.cache", shader->shader_hash);

	VkPipelineCache vk_pipeline_cache =
			_load_pipeline_cache(device, cache_path, physical_device_properties);

	VkPipeline vk_pipeline = VK_NULL_HANDLE;
	VK_CHECK(vkCreateComputePipelines(
			device, vk_pipeline_cache, 1, &create_info, nullptr, &vk_pipeline));

	VulkanPipeline* pipeline = VersatileResource::allocate<VulkanPipeline>(resources_allocator);
	pipeline->vk_pipeline = vk_pipeline;
	pipeline->vk_pipeline_cache = vk_pipeline_cache;
	pipeline->shader_hash = shader->shader_hash;

	return Pipeline(pipeline);
}

void VulkanRenderBackend::pipeline_free(Pipeline p_pipeline) {
	VulkanPipeline* pipeline = (VulkanPipeline*)p_pipeline;

	// save the pipeline cache
	if (pipeline->vk_pipeline_cache != VK_NULL_HANDLE) {
		size_t cache_size;
		VK_CHECK(vkGetPipelineCacheData(device, pipeline->vk_pipeline_cache, &cache_size, nullptr));

		std::vector<char> cache_data(cache_size);
		VK_CHECK(vkGetPipelineCacheData(
				device, pipeline->vk_pipeline_cache, &cache_size, cache_data.data()));

		fs::path path = std::format(".glitch/cache/{}.cache", pipeline->shader_hash);

		// if directory not exists create it
		if (!fs::exists(path.parent_path())) {
			// TODO: this should always be at the executable directory
			fs::create_directories(path.parent_path());
		}

		std::ofstream file(path, std::ios::binary);
		if (file) {
			// write header for integrity
			PipelineCacheHeader header = {};
			header.magic_number = PIPELINE_CACHE_MAGIC_NUMBER;
			header.data_size = cache_size;
			header.vendor_id = physical_device_properties.vendorID;
			header.device_id = physical_device_properties.deviceID;
			header.driver_version = physical_device_properties.driverVersion;
			memcpy(header.uuid, physical_device_properties.pipelineCacheUUID,
					VK_UUID_SIZE * sizeof(char));

			file.write((const char*)&header, sizeof(PipelineCacheHeader));

			file.write(cache_data.data(), cache_size);
		} else {
			GL_LOG_ERROR("[VULKAN] [VulkanRenderBackend::pipeline_free] Unable to write pipeline "
						 "cache data to file!");
		}
	}

	vkDestroyPipeline(device, pipeline->vk_pipeline, nullptr);
	vkDestroyPipelineCache(device, pipeline->vk_pipeline_cache, nullptr);

	VersatileResource::free(resources_allocator, pipeline);
}

} //namespace gl