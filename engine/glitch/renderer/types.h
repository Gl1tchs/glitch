/**
 * @file types.h
 */

#pragma once

#include "glitch/core/color.h"

// defines handles that are not needed to be freed by user
#define GL_DEFINE_HANDLE(object) typedef struct object##_T* object;

// defines handles that are needed to be freed by user
#define GL_DEFINE_NON_DISPATCHABLE_HANDLE(object)                              \
	typedef struct object##_T* object;

GL_DEFINE_NON_DISPATCHABLE_HANDLE(Buffer)
GL_DEFINE_NON_DISPATCHABLE_HANDLE(Image)
GL_DEFINE_NON_DISPATCHABLE_HANDLE(Sampler)
GL_DEFINE_NON_DISPATCHABLE_HANDLE(CommandPool)
GL_DEFINE_HANDLE(CommandBuffer)
GL_DEFINE_NON_DISPATCHABLE_HANDLE(CommandQueue)
GL_DEFINE_NON_DISPATCHABLE_HANDLE(RenderPass)
GL_DEFINE_NON_DISPATCHABLE_HANDLE(FrameBuffer)
GL_DEFINE_NON_DISPATCHABLE_HANDLE(Swapchain)
GL_DEFINE_NON_DISPATCHABLE_HANDLE(Pipeline)
GL_DEFINE_NON_DISPATCHABLE_HANDLE(Shader)
GL_DEFINE_NON_DISPATCHABLE_HANDLE(UniformSet)
GL_DEFINE_NON_DISPATCHABLE_HANDLE(Fence)
GL_DEFINE_NON_DISPATCHABLE_HANDLE(Semaphore)

#define GL_NULL_HANDLE nullptr
#define GL_REMAINING_MIP_LEVELS (~0U)
#define GL_REMAINING_ARRAY_LAYERS (~0U)

/**
 * Enum representing data formats in GPU
 */
enum DataFormat {
	DATA_FORMAT_UNDEFINED = 0,
	DATA_FORMAT_R8_UNORM = 9,
	DATA_FORMAT_R8_SNORM = 10,
	DATA_FORMAT_R8_USCALED = 11,
	DATA_FORMAT_R8_SSCALED = 12,
	DATA_FORMAT_R8_UINT = 13,
	DATA_FORMAT_R8_SINT = 14,
	DATA_FORMAT_R8_SRGB = 15,
	DATA_FORMAT_R8G8_UNORM = 16,
	DATA_FORMAT_R8G8_SNORM = 17,
	DATA_FORMAT_R8G8_USCALED = 18,
	DATA_FORMAT_R8G8_SSCALED = 19,
	DATA_FORMAT_R8G8_UINT = 20,
	DATA_FORMAT_R8G8_SINT = 21,
	DATA_FORMAT_R8G8_SRGB = 22,
	DATA_FORMAT_R8G8B8_UNORM = 23,
	DATA_FORMAT_R8G8B8_SNORM = 24,
	DATA_FORMAT_R8G8B8_USCALED = 25,
	DATA_FORMAT_R8G8B8_SSCALED = 26,
	DATA_FORMAT_R8G8B8_UINT = 27,
	DATA_FORMAT_R8G8B8_SINT = 28,
	DATA_FORMAT_R8G8B8_SRGB = 29,
	DATA_FORMAT_B8G8R8_UNORM = 30,
	DATA_FORMAT_B8G8R8_SNORM = 31,
	DATA_FORMAT_B8G8R8_USCALED = 32,
	DATA_FORMAT_B8G8R8_SSCALED = 33,
	DATA_FORMAT_B8G8R8_UINT = 34,
	DATA_FORMAT_B8G8R8_SINT = 35,
	DATA_FORMAT_B8G8R8_SRGB = 36,
	DATA_FORMAT_R8G8B8A8_UNORM = 37,
	DATA_FORMAT_R8G8B8A8_SNORM = 38,
	DATA_FORMAT_R8G8B8A8_USCALED = 39,
	DATA_FORMAT_R8G8B8A8_SSCALED = 40,
	DATA_FORMAT_R8G8B8A8_UINT = 41,
	DATA_FORMAT_R8G8B8A8_SINT = 42,
	DATA_FORMAT_R8G8B8A8_SRGB = 43,
	DATA_FORMAT_B8G8R8A8_UNORM = 44,
	DATA_FORMAT_B8G8R8A8_SNORM = 45,
	DATA_FORMAT_B8G8R8A8_USCALED = 46,
	DATA_FORMAT_B8G8R8A8_SSCALED = 47,
	DATA_FORMAT_B8G8R8A8_UINT = 48,
	DATA_FORMAT_B8G8R8A8_SINT = 49,
	DATA_FORMAT_B8G8R8A8_SRGB = 50,
	DATA_FORMAT_A8B8G8R8_UNORM_PACK32 = 51,
	DATA_FORMAT_A8B8G8R8_SNORM_PACK32 = 52,
	DATA_FORMAT_A8B8G8R8_USCALED_PACK32 = 53,
	DATA_FORMAT_A8B8G8R8_SSCALED_PACK32 = 54,
	DATA_FORMAT_A8B8G8R8_UINT_PACK32 = 55,
	DATA_FORMAT_A8B8G8R8_SINT_PACK32 = 56,
	DATA_FORMAT_A8B8G8R8_SRGB_PACK32 = 57,
	DATA_FORMAT_R16_UNORM = 70,
	DATA_FORMAT_R16_SNORM = 71,
	DATA_FORMAT_R16_USCALED = 72,
	DATA_FORMAT_R16_SSCALED = 73,
	DATA_FORMAT_R16_UINT = 74,
	DATA_FORMAT_R16_SINT = 75,
	DATA_FORMAT_R16_SFLOAT = 76,
	DATA_FORMAT_R16G16_UNORM = 77,
	DATA_FORMAT_R16G16_SNORM = 78,
	DATA_FORMAT_R16G16_USCALED = 79,
	DATA_FORMAT_R16G16_SSCALED = 80,
	DATA_FORMAT_R16G16_UINT = 81,
	DATA_FORMAT_R16G16_SINT = 82,
	DATA_FORMAT_R16G16_SFLOAT = 83,
	DATA_FORMAT_R16G16B16_UNORM = 84,
	DATA_FORMAT_R16G16B16_SNORM = 85,
	DATA_FORMAT_R16G16B16_USCALED = 86,
	DATA_FORMAT_R16G16B16_SSCALED = 87,
	DATA_FORMAT_R16G16B16_UINT = 88,
	DATA_FORMAT_R16G16B16_SINT = 89,
	DATA_FORMAT_R16G16B16_SFLOAT = 90,
	DATA_FORMAT_R16G16B16A16_UNORM = 91,
	DATA_FORMAT_R16G16B16A16_SNORM = 92,
	DATA_FORMAT_R16G16B16A16_USCALED = 93,
	DATA_FORMAT_R16G16B16A16_SSCALED = 94,
	DATA_FORMAT_R16G16B16A16_UINT = 95,
	DATA_FORMAT_R16G16B16A16_SINT = 96,
	DATA_FORMAT_R16G16B16A16_SFLOAT = 97,
	DATA_FORMAT_R32_UINT = 98,
	DATA_FORMAT_R32_SINT = 99,
	DATA_FORMAT_R32_SFLOAT = 100,
	DATA_FORMAT_R32G32_UINT = 101,
	DATA_FORMAT_R32G32_SINT = 102,
	DATA_FORMAT_R32G32_SFLOAT = 103,
	DATA_FORMAT_R32G32B32_UINT = 104,
	DATA_FORMAT_R32G32B32_SINT = 105,
	DATA_FORMAT_R32G32B32_SFLOAT = 106,
	DATA_FORMAT_R32G32B32A32_UINT = 107,
	DATA_FORMAT_R32G32B32A32_SINT = 108,
	DATA_FORMAT_R32G32B32A32_SFLOAT = 109,
	DATA_FORMAT_D16_UNORM = 124,
	DATA_FORMAT_D16_UNORM_S8_UINT = 128,
	DATA_FORMAT_D24_UNORM_S8_UINT = 129,
	DATA_FORMAT_D32_SFLOAT = 126,
	DATA_FORMAT_MAX = 0x7FFFFFFF,
};

GL_API size_t get_data_format_size(DataFormat p_format);

enum CompareOperator {
	COMPARE_OP_NEVER,
	COMPARE_OP_LESS,
	COMPARE_OP_EQUAL,
	COMPARE_OP_LESS_OR_EQUAL,
	COMPARE_OP_GREATER,
	COMPARE_OP_NOT_EQUAL,
	COMPARE_OP_GREATER_OR_EQUAL,
	COMPARE_OP_ALWAYS,
	COMPARE_OP_MAX
};

enum MemoryAllocationType {
	MEMORY_ALLOCATION_TYPE_CPU,
	MEMORY_ALLOCATION_TYPE_GPU,
};

enum BufferUsageBits {
	BUFFER_USAGE_TRANSFER_SRC_BIT = 0x00000001,
	BUFFER_USAGE_TRANSFER_DST_BIT = 0x00000002,
	BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT = 0x00000004,
	BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT = 0x00000008,
	BUFFER_USAGE_UNIFORM_BUFFER_BIT = 0x00000010,
	BUFFER_USAGE_STORAGE_BUFFER_BIT = 0x00000020,
	BUFFER_USAGE_INDEX_BUFFER_BIT = 0x00000040,
	BUFFER_USAGE_VERTEX_BUFFER_BIT = 0x00000080,
	BUFFER_USAGE_INDIRECT_BUFFER_BIT = 0x00000100,
	BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT = 0x00020000,
};

enum ImageLayout {
	IMAGE_LAYOUT_UNDEFINED = 0,
	IMAGE_LAYOUT_GENERAL = 1,
	IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL = 2,
	IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL = 3,
	IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL = 4,
	IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL = 5,
	IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL = 6,
	IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL = 7,
	IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL = 1000117000,
	IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL = 1000117001,
	IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL = 1000241000,
	IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL = 1000241001,
	IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL = 1000241002,
	IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL = 1000241003,
	IMAGE_LAYOUT_READ_ONLY_OPTIMAL = 1000314000,
	IMAGE_LAYOUT_ATTACHMENT_OPTIMAL = 1000314001,
	IMAGE_LAYOUT_PRESENT_SRC = 1000001002,
	IMAGE_LAYOUT_MAX_ENUM = 0x7FFFFFFF
};

enum ImageFiltering {
	IMAGE_FILTERING_NEAREST = 0,
	IMAGE_FILTERING_LINEAR = 1,
};

enum ImageWrappingMode {
	IMAGE_WRAPPING_MODE_REPEAT = 0,
	IMAGE_WRAPPING_MODE_MIRRORED_REPEAT = 1,
	IMAGE_WRAPPING_MODE_CLAMP_TO_EDGE = 2,
	IMAGE_WRAPPING_MODE_CLAMP_TO_BORDER = 3,
	IMAGE_WRAPPING_MODE_MIRROR_CLAMP_TO_EDGE = 4,
	IMAGE_WRAPPING_MODE_MAX_ENUM = 0x7FFFFFFF
};

enum ImageAspectFlags {
	IMAGE_ASPECT_COLOR_BIT = 0x00000001,
	IMAGE_ASPECT_DEPTH_BIT = 0x00000002,
	IMAGE_ASPECT_STENCIL_BIT = 0x00000004,
};

struct ImageSubresourceLayers {
	ImageAspectFlags aspect_mask;
	uint32_t mip_level;
	uint32_t base_array_layer;
	uint32_t layer_count;
};

enum ResolveMode {
	RESOLVE_MODE_NONE = 0,
	RESOLVE_MODE_SAMPLE_ZERO_BIT = 0x00000001,
	RESOLVE_MODE_AVERAGE_BIT = 0x00000002,
	RESOLVE_MODE_MIN_BIT = 0x00000004,
	RESOLVE_MODE_MAX_BIT = 0x00000008,
};

struct ImageResolve {
	ImageSubresourceLayers src_subresource;
	glm::ivec3 src_offset;
	ImageSubresourceLayers dst_subresource;
	glm::ivec3 dst_offset;
	glm::uvec3 extent;
};

enum ImageSamples {
	IMAGE_SAMPLES_1 = 0x00000001,
	IMAGE_SAMPLES_2 = 0x00000002,
	IMAGE_SAMPLES_4 = 0x00000004,
	IMAGE_SAMPLES_8 = 0x00000008,
	IMAGE_SAMPLES_16 = 0x00000010,
	IMAGE_SAMPLES_32 = 0x00000020,
	IMAGE_SAMPLES_64 = 0x00000040,
	IMAGE_SAMPLES_MAX = 0x7FFFFFFF,
};

enum ImageUsageBits {
	IMAGE_USAGE_TRANSFER_SRC_BIT = 0x00000001,
	IMAGE_USAGE_TRANSFER_DST_BIT = 0x00000002,
	IMAGE_USAGE_SAMPLED_BIT = 0x00000004,
	IMAGE_USAGE_STORAGE_BIT = 0x00000008,
	IMAGE_USAGE_COLOR_ATTACHMENT_BIT = 0x00000010,
	IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT = 0x00000020,
};

enum QueueType {
	QUEUE_TYPE_GRAPHICS,
	QUEUE_TYPE_PRESENT,
	QUEUE_TYPE_TRANSFER,
};

enum IndexType {
	INDEX_TYPE_UINT16 = 0x00000001,
	INDEX_TYPE_UINT32 = 0x00000002,
	INDEX_TYPE_MAX = 0xffffffff,
};

enum PipelineType {
	PIPELINE_TYPE_GRAPHICS,
	PIPELINE_TYPE_COMPUTE,
};

struct BufferCopyRegion {
	uint64_t src_offset;
	uint64_t dst_offset;
	uint64_t size;
};

struct BufferImageCopyRegion {
	uint64_t buffer_offset;
	uint32_t buffer_row_length;
	uint32_t buffer_image_height;
	ImageSubresourceLayers image_subresource;
	glm::uvec3 image_offset;
	glm::uvec3 image_extent;
};

inline const uint32_t MAX_UNIFORM_SETS = 16;

enum ShaderUniformType {
	UNIFORM_TYPE_SAMPLER, // For sampling only (sampler GLSL type).
	UNIFORM_TYPE_SAMPLER_WITH_TEXTURE, // For sampling only, but includes a
									   // texture, (samplerXX GLSL type),
									   // first a sampler then a texture.
	UNIFORM_TYPE_TEXTURE, // Only texture, (textureXX GLSL type).
	UNIFORM_TYPE_IMAGE, // Storage image (imageXX GLSL type), for compute
						// mostly.
	UNIFORM_TYPE_UNIFORM_BUFFER, // Regular uniform buffer (or UBO).
	UNIFORM_TYPE_STORAGE_BUFFER, // Storage buffer ("buffer" qualifier) like
								 // UBO, but supports storage, for compute
								 // mostly.
	UNIFORM_TYPE_MAX
};

/**
 * Struct representing a single shader uniform that can be a type of
 * `ShaderUniformType` in specified `binding`
 */
struct ShaderUniform {
	ShaderUniformType type = UNIFORM_TYPE_MAX;
	uint32_t binding = 0xffffffff; // Binding index as specified in shader.
	std::vector<void*> data;
};

enum AttachmentLoadOp {
	ATTACHMENT_LOAD_OP_LOAD = 0,
	ATTACHMENT_LOAD_OP_CLEAR = 1,
	ATTACHMENT_LOAD_OP_DONT_CARE = 2,
	ATTACHMENT_LOAD_OP_NONE = 1000400000,
	ATTACHMENT_LOAD_OP_MAX_ENUM = 0x7FFFFFFF
};

enum AttachmentStoreOp {
	ATTACHMENT_STORE_OP_STORE = 0,
	ATTACHMENT_STORE_OP_DONT_CARE = 1,
	ATTACHMENT_STORE_OP_NONE = 1000301000,
	ATTACHMENT_STORE_OP_MAX_ENUM = 0x7FFFFFFF
};

struct RenderPassAttachment {
	DataFormat format;
	AttachmentLoadOp load_op = ATTACHMENT_LOAD_OP_CLEAR;
	AttachmentStoreOp store_op = ATTACHMENT_STORE_OP_STORE;
	ImageLayout final_layout = IMAGE_LAYOUT_UNDEFINED;
	// TODO: implement
	uint32_t sample_count = 1;
	bool is_depth_attachment = false;
};

enum SubpassAttachmentType {
	SUBPASS_ATTACHMENT_COLOR,
	SUBPASS_ATTACHMENT_DEPTH_STENCIL,
	SUBPASS_ATTACHMENT_INPUT,
};

struct SubpassAttachment {
	uint32_t attachment_index;
	SubpassAttachmentType type;
};

struct SubpassInfo {
	std::vector<SubpassAttachment> attachments;
};

struct RenderingAttachment {
	Image image;
	ImageLayout layout = IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	AttachmentLoadOp load_op = ATTACHMENT_LOAD_OP_DONT_CARE;
	AttachmentStoreOp store_op = ATTACHMENT_STORE_OP_STORE;
	// load_op must be set to ATTACHMENT_LOAD_OP_CLEAR
	Color clear_color = COLOR_BLACK;

	// For MSAA
	ResolveMode resolve_mode = RESOLVE_MODE_NONE;
	Image resolve_image = GL_NULL_HANDLE;
	ImageLayout resolve_layout = IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
};

enum RenderPrimitive {
	RENDER_PRIMITIVE_POINTS,
	RENDER_PRIMITIVE_LINES,
	RENDER_PRIMITIVE_LINES_WITH_ADJACENCY,
	RENDER_PRIMITIVE_LINESTRIPS,
	RENDER_PRIMITIVE_LINESTRIPS_WITH_ADJACENCY,
	RENDER_PRIMITIVE_TRIANGLES,
	RENDER_PRIMITIVE_TRIANGLES_WITH_ADJACENCY,
	RENDER_PRIMITIVE_TRIANGLE_STRIPS,
	RENDER_PRIMITIVE_TRIANGLE_STRIPS_WITH_AJACENCY,
	RENDER_PRIMITIVE_TRIANGLE_STRIPS_WITH_RESTART_INDEX,
	RENDER_PRIMITIVE_TESSELATION_PATCH,
	RENDER_PRIMITIVE_MAX
};

enum PolygonCullMode {
	POLYGON_CULL_DISABLED,
	POLYGON_CULL_FRONT,
	POLYGON_CULL_BACK,
	POLYGON_CULL_MAX
};

enum PolygonFrontFace {
	POLYGON_FRONT_FACE_CLOCKWISE,
	POLYGON_FRONT_FACE_COUNTER_CLOCKWISE,
};

enum StencilOperator {
	STENCIL_OP_KEEP,
	STENCIL_OP_ZERO,
	STENCIL_OP_REPLACE,
	STENCIL_OP_INCREMENT_AND_CLAMP,
	STENCIL_OP_DECREMENT_AND_CLAMP,
	STENCIL_OP_INVERT,
	STENCIL_OP_INCREMENT_AND_WRAP,
	STENCIL_OP_DECREMENT_AND_WRAP,
	STENCIL_OP_MAX
};

enum LogicOperator {
	LOGIC_OP_CLEAR,
	LOGIC_OP_AND,
	LOGIC_OP_AND_REVERSE,
	LOGIC_OP_COPY,
	LOGIC_OP_AND_INVERTED,
	LOGIC_OP_NO_OP,
	LOGIC_OP_XOR,
	LOGIC_OP_OR,
	LOGIC_OP_NOR,
	LOGIC_OP_EQUIVALENT,
	LOGIC_OP_INVERT,
	LOGIC_OP_OR_REVERSE,
	LOGIC_OP_COPY_INVERTED,
	LOGIC_OP_OR_INVERTED,
	LOGIC_OP_NAND,
	LOGIC_OP_SET,
	LOGIC_OP_MAX
};

enum BlendFactor {
	BLEND_FACTOR_ZERO,
	BLEND_FACTOR_ONE,
	BLEND_FACTOR_SRC_COLOR,
	BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
	BLEND_FACTOR_DST_COLOR,
	BLEND_FACTOR_ONE_MINUS_DST_COLOR,
	BLEND_FACTOR_SRC_ALPHA,
	BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
	BLEND_FACTOR_DST_ALPHA,
	BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
	BLEND_FACTOR_CONSTANT_COLOR,
	BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR,
	BLEND_FACTOR_CONSTANT_ALPHA,
	BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA,
	BLEND_FACTOR_SRC_ALPHA_SATURATE,
	BLEND_FACTOR_SRC1_COLOR,
	BLEND_FACTOR_ONE_MINUS_SRC1_COLOR,
	BLEND_FACTOR_SRC1_ALPHA,
	BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA,
	BLEND_FACTOR_MAX
};

enum BlendOperation {
	BLEND_OP_ADD,
	BLEND_OP_SUBTRACT,
	BLEND_OP_REVERSE_SUBTRACT,
	BLEND_OP_MINIMUM,
	BLEND_OP_MAXIMUM,
	BLEND_OP_MAX
};

struct ShaderInterfaceVariable {
	const char* name;
	uint32_t location;
	DataFormat format;
};

struct PipelineVertexInputState {
	uint32_t stride = 0;
};

struct PipelineRasterizationState {
	bool enable_depth_clamp = false;
	bool discard_primitives = false;
	bool wireframe = false;
	PolygonCullMode cull_mode = POLYGON_CULL_DISABLED;
	PolygonFrontFace front_face = POLYGON_FRONT_FACE_CLOCKWISE;
	bool depth_bias_enabled = false;
	float depth_bias_constant_factor = 0.0f;
	float depth_bias_clamp = 0.0f;
	float depth_bias_slope_factor = 0.0f;
	float line_width = 1.0f;
};

struct PipelineMultisampleState {
	ImageSamples sample_count = IMAGE_SAMPLES_1;
	bool enable_sample_shading = false;
	float min_sample_shading = 0.0f;
	std::vector<uint32_t> sample_mask;
	bool enable_alpha_to_coverage = false;
	bool enable_alpha_to_one = false;
};

struct PipelineDepthStencilState {
	bool enable_depth_test = false;
	bool enable_depth_write = false;
	CompareOperator depth_compare_operator = COMPARE_OP_ALWAYS;
	bool enable_depth_range = false;
	float depth_range_min = 0;
	float depth_range_max = 1.0f;
	bool enable_stencil = false;

	struct StencilOperationState {
		StencilOperator fail = STENCIL_OP_ZERO;
		StencilOperator pass = STENCIL_OP_ZERO;
		StencilOperator depth_fail = STENCIL_OP_ZERO;
		CompareOperator compare = COMPARE_OP_ALWAYS;
		uint32_t compare_mask = 0;
		uint32_t write_mask = 0;
		uint32_t reference = 0;
	};

	StencilOperationState front_op;
	StencilOperationState back_op;
};

struct PipelineColorBlendState {
	bool enable_logic_op = false;
	LogicOperator logic_op = LOGIC_OP_CLEAR;

	struct Attachment {
		bool enable_blend = false;
		BlendFactor src_color_blend_factor = BLEND_FACTOR_ZERO;
		BlendFactor dst_color_blend_factor = BLEND_FACTOR_ZERO;
		BlendOperation color_blend_op = BLEND_OP_ADD;
		BlendFactor src_alpha_blend_factor = BLEND_FACTOR_ZERO;
		BlendFactor dst_alpha_blend_factor = BLEND_FACTOR_ZERO;
		BlendOperation alpha_blend_op = BLEND_OP_ADD;
		bool write_r = true;
		bool write_g = true;
		bool write_b = true;
		bool write_a = true;
	};

	static PipelineColorBlendState create_disabled(int p_attachments = 1) {
		PipelineColorBlendState bs;
		for (int i = 0; i < p_attachments; i++) {
			bs.attachments.push_back(Attachment());
		}
		return bs;
	}

	static PipelineColorBlendState create_blend(int p_attachments = 1) {
		PipelineColorBlendState bs;
		for (int i = 0; i < p_attachments; i++) {
			Attachment ba;
			ba.enable_blend = true;
			ba.src_color_blend_factor = BLEND_FACTOR_SRC_ALPHA;
			ba.dst_color_blend_factor = BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			ba.src_alpha_blend_factor = BLEND_FACTOR_SRC_ALPHA;
			ba.dst_alpha_blend_factor = BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;

			bs.attachments.push_back(ba);
		}
		return bs;
	}

	std::vector<Attachment> attachments; // One per render target texture.
	glm::vec4 blend_constant;
};

struct RenderingState {
	std::vector<DataFormat> color_attachments;
	DataFormat depth_attachment;
};

enum PipelineDynamicStateFlags {
	DYNAMIC_STATE_LINE_WIDTH = (1 << 0),
	DYNAMIC_STATE_DEPTH_BIAS = (1 << 1),
	DYNAMIC_STATE_BLEND_CONSTANTS = (1 << 2),
	DYNAMIC_STATE_DEPTH_BOUNDS = (1 << 3),
	DYNAMIC_STATE_STENCIL_COMPARE_MASK = (1 << 4),
	DYNAMIC_STATE_STENCIL_WRITE_MASK = (1 << 5),
	DYNAMIC_STATE_STENCIL_REFERENCE = (1 << 6),
};

enum ShaderStage {
	SHADER_STAGE_VERTEX_BIT = 0x00000001,
	SHADER_STAGE_TESSELLATION_CONTROL_BIT = 0x00000002,
	SHADER_STAGE_TESSELLATION_EVALUATION_BIT = 0x00000004,
	SHADER_STAGE_GEOMETRY_BIT = 0x00000008,
	SHADER_STAGE_FRAGMENT_BIT = 0x00000010,
	SHADER_STAGE_COMPUTE_BIT = 0x00000020,
	SHADER_STAGE_ALL_GRAPHICS = 0x0000001F,
	SHADER_STAGE_ALL = 0x7FFFFFFF,
};

struct SpirvData {
	std::vector<uint32_t> byte_code;
	ShaderStage stage;
};

typedef uint64_t BufferDeviceAddress;

struct DrawIndirectData {
	uint32_t vertex_count;
	uint32_t instance_count = 1;
	uint32_t first_vertex = 0;
	uint32_t first_instance = 1;
};

struct DrawIndirectIndexedData {
	uint32_t index_count;
	uint32_t instance_count = 1;
	uint32_t first_index = 0;
	int32_t vertex_offset = 0;
	uint32_t first_instance = 0;
};
