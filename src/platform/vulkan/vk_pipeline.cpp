#include "platform/vulkan/vk_backend.h"
#include "glitch/renderer/types.h"
#include <vulkan/vulkan_core.h>
#include <algorithm>

static const VkPrimitiveTopology GL_TO_VK_PRIMITIVE[RENDER_PRIMITIVE_MAX] = {
	VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
	VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
	VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY,
	VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,
	VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY,
	VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
	VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY,
	VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
	VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY,
	VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
	VK_PRIMITIVE_TOPOLOGY_PATCH_LIST,
};

static const VkCompareOp GL_TO_VK_COMPARE_OP[COMPARE_OP_MAX] = {
	VK_COMPARE_OP_NEVER,
	VK_COMPARE_OP_LESS,
	VK_COMPARE_OP_EQUAL,
	VK_COMPARE_OP_LESS_OR_EQUAL,
	VK_COMPARE_OP_GREATER,
	VK_COMPARE_OP_NOT_EQUAL,
	VK_COMPARE_OP_GREATER_OR_EQUAL,
	VK_COMPARE_OP_ALWAYS,
};

static const VkStencilOp GL_TO_VK_STENCIL_OP[STENCIL_OP_MAX] = {
	VK_STENCIL_OP_KEEP,
	VK_STENCIL_OP_ZERO,
	VK_STENCIL_OP_REPLACE,
	VK_STENCIL_OP_INCREMENT_AND_CLAMP,
	VK_STENCIL_OP_DECREMENT_AND_CLAMP,
	VK_STENCIL_OP_INVERT,
	VK_STENCIL_OP_INCREMENT_AND_WRAP,
	VK_STENCIL_OP_DECREMENT_AND_WRAP,
};

static const VkLogicOp GL_TO_VK_LOGIC_OP[LOGIC_OP_MAX] = {
	VK_LOGIC_OP_CLEAR,
	VK_LOGIC_OP_AND,
	VK_LOGIC_OP_AND_REVERSE,
	VK_LOGIC_OP_COPY,
	VK_LOGIC_OP_AND_INVERTED,
	VK_LOGIC_OP_NO_OP,
	VK_LOGIC_OP_XOR,
	VK_LOGIC_OP_OR,
	VK_LOGIC_OP_NOR,
	VK_LOGIC_OP_EQUIVALENT,
	VK_LOGIC_OP_INVERT,
	VK_LOGIC_OP_OR_REVERSE,
	VK_LOGIC_OP_COPY_INVERTED,
	VK_LOGIC_OP_OR_INVERTED,
	VK_LOGIC_OP_NAND,
	VK_LOGIC_OP_SET,
};

static const VkBlendFactor GL_TO_VK_BLEND_FACTOR[BLEND_FACTOR_MAX] = {
	VK_BLEND_FACTOR_ZERO,
	VK_BLEND_FACTOR_ONE,
	VK_BLEND_FACTOR_SRC_COLOR,
	VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
	VK_BLEND_FACTOR_DST_COLOR,
	VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,
	VK_BLEND_FACTOR_SRC_ALPHA,
	VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
	VK_BLEND_FACTOR_DST_ALPHA,
	VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
	VK_BLEND_FACTOR_CONSTANT_COLOR,
	VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR,
	VK_BLEND_FACTOR_CONSTANT_ALPHA,
	VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA,
	VK_BLEND_FACTOR_SRC_ALPHA_SATURATE,
	VK_BLEND_FACTOR_SRC1_COLOR,
	VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR,
	VK_BLEND_FACTOR_SRC1_ALPHA,
	VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA,
};

static const VkBlendOp GL_TO_VK_BLEND_OP[BLEND_OP_MAX] = {
	VK_BLEND_OP_ADD,
	VK_BLEND_OP_SUBTRACT,
	VK_BLEND_OP_REVERSE_SUBTRACT,
	VK_BLEND_OP_MIN,
	VK_BLEND_OP_MAX,
};

static VkCullModeFlagBits _gl_to_vk_cull_mode(PolygonCullMode cull_mode) {
	switch (cull_mode) {
		case POLYGON_CULL_DISABLED:
			return VK_CULL_MODE_NONE;
		case POLYGON_CULL_FRONT:
			return VK_CULL_MODE_FRONT_BIT;
		case POLYGON_CULL_BACK:
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

static VkPipelineCache _load_pipeline_cache(VkDevice p_device,
		const fs::path& p_path,
		const VkPhysicalDeviceProperties& p_device_props) {
	VkPipelineCacheCreateInfo cache_create_info = {};
	cache_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

	// if cache already exists on disk try to load it
	const bool cache_exists = fs::exists(p_path);
	if (cache_exists) {
		std::ifstream file(p_path, std::ios::binary);
		if (!file) {
			GL_LOG_ERROR("Unable to parse pipeline cache: {}", p_path.string());
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
						reinterpret_cast<const PipelineCacheHeader*>(
								cache.data());

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
					if (header->uuid[i] !=
							p_device_props.pipelineCacheUUID[i]) {
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
	VK_CHECK(vkCreatePipelineCache(
			p_device, &cache_create_info, nullptr, &vk_pipeline_cache));

	return vk_pipeline_cache;
}

Pipeline VulkanRenderBackend::render_pipeline_create(Shader p_shader,
		RenderPrimitive p_render_primitive,
		PipelineVertexInputState p_vertex_input_state,
		PipelineRasterizationState p_rasterization_state,
		PipelineMultisampleState p_multisample_state,
		PipelineDepthStencilState p_depth_stencil_state,
		PipelineColorBlendState p_blend_state,
		BitField<PipelineDynamicStateFlags> p_dynamic_state,
		RenderingState p_rendering_state) {
	// input assembly state
	VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
	input_assembly.sType =
			VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly.topology = GL_TO_VK_PRIMITIVE[p_render_primitive];
	input_assembly.primitiveRestartEnable = false;

	// rasterizer state
	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType =
			VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = p_rasterization_state.enable_depth_clamp;
	rasterizer.rasterizerDiscardEnable =
			p_rasterization_state.discard_primitives;
	rasterizer.polygonMode = p_rasterization_state.wireframe
			? VK_POLYGON_MODE_LINE
			: VK_POLYGON_MODE_FILL;
	rasterizer.cullMode = _gl_to_vk_cull_mode(p_rasterization_state.cull_mode);
	rasterizer.frontFace =
			p_rasterization_state.front_face == POLYGON_FRONT_FACE_CLOCKWISE
			? VK_FRONT_FACE_CLOCKWISE
			: VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = p_rasterization_state.depth_bias_enabled;
	rasterizer.depthBiasClamp = p_rasterization_state.depth_bias_clamp;
	rasterizer.depthBiasSlopeFactor =
			p_rasterization_state.depth_bias_slope_factor;
	rasterizer.lineWidth = p_rasterization_state.line_width;

	// multisampling state
	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType =
			VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.rasterizationSamples = static_cast<VkSampleCountFlagBits>(
			p_multisample_state.sample_count);
	multisampling.sampleShadingEnable =
			p_multisample_state.enable_sample_shading;
	multisampling.minSampleShading = p_multisample_state.min_sample_shading;
	multisampling.pSampleMask =
			(VkSampleMask*)p_multisample_state.sample_mask.data();
	multisampling.alphaToCoverageEnable =
			p_multisample_state.enable_alpha_to_coverage;
	multisampling.alphaToOneEnable = p_multisample_state.enable_alpha_to_one;

	// depth stencil state
	VkPipelineDepthStencilStateCreateInfo depth_stencil = {};
	depth_stencil.sType =
			VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depth_stencil.depthTestEnable = p_depth_stencil_state.enable_depth_test;
	depth_stencil.depthWriteEnable = p_depth_stencil_state.enable_depth_write;
	depth_stencil.depthCompareOp =
			GL_TO_VK_COMPARE_OP[p_depth_stencil_state.depth_compare_operator];
	depth_stencil.depthBoundsTestEnable =
			p_depth_stencil_state.enable_depth_range;
	depth_stencil.minDepthBounds = p_depth_stencil_state.depth_range_min;
	depth_stencil.maxDepthBounds = p_depth_stencil_state.depth_range_max;
	depth_stencil.stencilTestEnable = p_depth_stencil_state.enable_stencil;

	VkStencilOpState front_stencil_state;
	front_stencil_state.failOp =
			GL_TO_VK_STENCIL_OP[p_depth_stencil_state.front_op.fail];
	front_stencil_state.passOp =
			GL_TO_VK_STENCIL_OP[p_depth_stencil_state.front_op.pass];
	front_stencil_state.depthFailOp =
			GL_TO_VK_STENCIL_OP[p_depth_stencil_state.front_op.depth_fail];
	front_stencil_state.compareOp =
			GL_TO_VK_COMPARE_OP[p_depth_stencil_state.front_op.compare];
	front_stencil_state.compareMask =
			p_depth_stencil_state.front_op.compare_mask;
	front_stencil_state.writeMask = p_depth_stencil_state.front_op.write_mask;
	front_stencil_state.reference = p_depth_stencil_state.front_op.reference;

	depth_stencil.front = front_stencil_state;

	VkStencilOpState back_stencil_state;
	back_stencil_state.failOp =
			GL_TO_VK_STENCIL_OP[p_depth_stencil_state.back_op.fail];
	back_stencil_state.passOp =
			GL_TO_VK_STENCIL_OP[p_depth_stencil_state.back_op.pass];
	back_stencil_state.depthFailOp =
			GL_TO_VK_STENCIL_OP[p_depth_stencil_state.back_op.depth_fail];
	back_stencil_state.compareOp =
			GL_TO_VK_COMPARE_OP[p_depth_stencil_state.back_op.compare];
	back_stencil_state.compareMask = p_depth_stencil_state.back_op.compare_mask;
	back_stencil_state.writeMask = p_depth_stencil_state.back_op.write_mask;
	back_stencil_state.reference = p_depth_stencil_state.back_op.reference;

	depth_stencil.back = back_stencil_state;

	// color blend attachment
	std::vector<VkPipelineColorBlendAttachmentState> color_blend_attachments;
	for (const auto& attachment : p_blend_state.attachments) {
		VkPipelineColorBlendAttachmentState vk_attachment = {};
		vk_attachment.blendEnable = attachment.enable_blend;
		vk_attachment.srcColorBlendFactor =
				GL_TO_VK_BLEND_FACTOR[attachment.src_color_blend_factor];
		vk_attachment.dstColorBlendFactor =
				GL_TO_VK_BLEND_FACTOR[attachment.dst_color_blend_factor];
		vk_attachment.colorBlendOp =
				GL_TO_VK_BLEND_OP[attachment.color_blend_op];
		vk_attachment.srcAlphaBlendFactor =
				GL_TO_VK_BLEND_FACTOR[attachment.src_alpha_blend_factor];
		vk_attachment.dstAlphaBlendFactor =
				GL_TO_VK_BLEND_FACTOR[attachment.dst_alpha_blend_factor];
		vk_attachment.alphaBlendOp =
				GL_TO_VK_BLEND_OP[attachment.alpha_blend_op];

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

	VkPipelineColorBlendStateCreateInfo color_blend = {};
	color_blend.sType =
			VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blend.logicOpEnable = p_blend_state.enable_logic_op;
	color_blend.logicOp = GL_TO_VK_LOGIC_OP[p_blend_state.logic_op];
	color_blend.attachmentCount =
			static_cast<uint32_t>(color_blend_attachments.size());
	color_blend.pAttachments = color_blend_attachments.data();

	color_blend.blendConstants[0] = p_blend_state.blend_constant.x;
	color_blend.blendConstants[1] = p_blend_state.blend_constant.y;
	color_blend.blendConstants[2] = p_blend_state.blend_constant.z;
	color_blend.blendConstants[3] = p_blend_state.blend_constant.w;

	// dynamic state
	std::vector<VkDynamicState> states = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
	};

	if (p_dynamic_state.has_flag(DYNAMIC_STATE_LINE_WIDTH)) {
		states.push_back(VK_DYNAMIC_STATE_LINE_WIDTH);
	}
	if (p_dynamic_state.has_flag(DYNAMIC_STATE_DEPTH_BIAS)) {
		states.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);
	}
	if (p_dynamic_state.has_flag(DYNAMIC_STATE_BLEND_CONSTANTS)) {
		states.push_back(VK_DYNAMIC_STATE_BLEND_CONSTANTS);
	}
	if (p_dynamic_state.has_flag(DYNAMIC_STATE_DEPTH_BOUNDS)) {
		states.push_back(VK_DYNAMIC_STATE_DEPTH_BOUNDS);
	}
	if (p_dynamic_state.has_flag(DYNAMIC_STATE_STENCIL_COMPARE_MASK)) {
		states.push_back(VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK);
	}
	if (p_dynamic_state.has_flag(DYNAMIC_STATE_STENCIL_WRITE_MASK)) {
		states.push_back(VK_DYNAMIC_STATE_STENCIL_WRITE_MASK);
	}
	if (p_dynamic_state.has_flag(DYNAMIC_STATE_STENCIL_REFERENCE)) {
		states.push_back(VK_DYNAMIC_STATE_STENCIL_REFERENCE);
	}

	VkPipelineDynamicStateCreateInfo dynamic_state = {};
	dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_state.dynamicStateCount = static_cast<uint32_t>(states.size());
	dynamic_state.pDynamicStates = states.data();

	// rendering state
	std::vector<VkFormat> vk_color_attachments;
	for (size_t i = 0; i < p_rendering_state.color_attachments.size(); i++) {
		vk_color_attachments.push_back(
				static_cast<VkFormat>(p_rendering_state.color_attachments[i]));
	}

	VkPipelineRenderingCreateInfo rendering_info = {};
	rendering_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
	rendering_info.colorAttachmentCount =
			static_cast<uint32_t>(vk_color_attachments.size());
	rendering_info.pColorAttachmentFormats = vk_color_attachments.data();
	rendering_info.depthAttachmentFormat =
			static_cast<VkFormat>(p_rendering_state.depth_attachment);

	VkVertexInputBindingDescription vertex_binding = {};
	vertex_binding.binding = 0;
	vertex_binding.stride = p_vertex_input_state.stride;
	vertex_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	std::vector<VkVertexInputAttributeDescription> vertex_attributes;
	for (const auto& input : shader_get_vertex_inputs(p_shader)) {
		VkVertexInputAttributeDescription attribute = {};
		attribute.binding = 0;
		attribute.format = static_cast<VkFormat>(input.format);
		attribute.location = input.location;
		attribute.offset = 0;

		vertex_attributes.push_back(attribute);
	}

	// attributes should be sorted by location then assign the offsets
	std::sort(vertex_attributes.begin(), vertex_attributes.end(),
			[](const auto& lhs, const auto& rhs) {
				return lhs.location < rhs.location;
			});

	uint32_t offset = 0;
	for (auto& attr : vertex_attributes) {
		size_t data_size =
				get_data_format_size(static_cast<DataFormat>(attr.format));
		GL_ASSERT(data_size != 0, "Unsupported data type to calculate size.");

		attr.offset = offset;
		offset += data_size;
	}

	VkPipelineVertexInputStateCreateInfo vertex_info = {};
	vertex_info.sType =
			VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_info.vertexBindingDescriptionCount = 1;
	vertex_info.pVertexBindingDescriptions = &vertex_binding;
	vertex_info.vertexAttributeDescriptionCount = vertex_attributes.size();
	vertex_info.pVertexAttributeDescriptions = vertex_attributes.data();

	VkPipelineViewportStateCreateInfo viewport_state = {};
	viewport_state.sType =
			VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state.viewportCount = 1;
	viewport_state.scissorCount = 1;

	VulkanShader* shader = (VulkanShader*)p_shader;

	VkGraphicsPipelineCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	create_info.pNext = &rendering_info,
	create_info.stageCount =
			static_cast<uint32_t>(shader->stage_create_infos.size());
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

	std::string cache_path =
			std::format(".glitch/cache/{}.cache", shader->shader_hash);

	VkPipelineCache vk_pipeline_cache = _load_pipeline_cache(
			device, cache_path, physical_device_properties);

	VkPipeline vk_pipeline = VK_NULL_HANDLE;
	VK_CHECK(vkCreateGraphicsPipelines(
			device, vk_pipeline_cache, 1, &create_info, nullptr, &vk_pipeline));

	VulkanPipeline* pipeline =
			VersatileResource::allocate<VulkanPipeline>(resources_allocator);
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

	std::string cache_path =
			std::format(".glitch/cache/{}.cache", shader->shader_hash);

	VkPipelineCache vk_pipeline_cache = _load_pipeline_cache(
			device, cache_path, physical_device_properties);

	VkPipeline vk_pipeline = VK_NULL_HANDLE;
	VK_CHECK(vkCreateComputePipelines(
			device, vk_pipeline_cache, 1, &create_info, nullptr, &vk_pipeline));

	VulkanPipeline* pipeline =
			VersatileResource::allocate<VulkanPipeline>(resources_allocator);
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
		VK_CHECK(vkGetPipelineCacheData(
				device, pipeline->vk_pipeline_cache, &cache_size, nullptr));

		std::vector<char> cache_data(cache_size);
		VK_CHECK(vkGetPipelineCacheData(device, pipeline->vk_pipeline_cache,
				&cache_size, cache_data.data()));

		fs::path path =
				std::format(".glitch/cache/{}.cache", pipeline->shader_hash);

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
			GL_LOG_ERROR("Unable to write pipeline cache data to file!");
		}
	}

	vkDestroyPipeline(device, pipeline->vk_pipeline, nullptr);
	vkDestroyPipelineCache(device, pipeline->vk_pipeline_cache, nullptr);

	VersatileResource::free(resources_allocator, pipeline);
}
