/**
 * @file types.h
 */

#pragma once

#include "glitch/core/color.h"

namespace gl {

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
enum class DataFormat : int {
	UNDEFINED = 0,
	R8_UNORM = 9,
	R8_SNORM = 10,
	R8_USCALED = 11,
	R8_SSCALED = 12,
	R8_UINT = 13,
	R8_SINT = 14,
	R8_SRGB = 15,
	R8G8_UNORM = 16,
	R8G8_SNORM = 17,
	R8G8_USCALED = 18,
	R8G8_SSCALED = 19,
	R8G8_UINT = 20,
	R8G8_SINT = 21,
	R8G8_SRGB = 22,
	R8G8B8_UNORM = 23,
	R8G8B8_SNORM = 24,
	R8G8B8_USCALED = 25,
	R8G8B8_SSCALED = 26,
	R8G8B8_UINT = 27,
	R8G8B8_SINT = 28,
	R8G8B8_SRGB = 29,
	B8G8R8_UNORM = 30,
	B8G8R8_SNORM = 31,
	B8G8R8_USCALED = 32,
	B8G8R8_SSCALED = 33,
	B8G8R8_UINT = 34,
	B8G8R8_SINT = 35,
	B8G8R8_SRGB = 36,
	R8G8B8A8_UNORM = 37,
	R8G8B8A8_SNORM = 38,
	R8G8B8A8_USCALED = 39,
	R8G8B8A8_SSCALED = 40,
	R8G8B8A8_UINT = 41,
	R8G8B8A8_SINT = 42,
	R8G8B8A8_SRGB = 43,
	B8G8R8A8_UNORM = 44,
	B8G8R8A8_SNORM = 45,
	B8G8R8A8_USCALED = 46,
	B8G8R8A8_SSCALED = 47,
	B8G8R8A8_UINT = 48,
	B8G8R8A8_SINT = 49,
	B8G8R8A8_SRGB = 50,
	A8B8G8R8_UNORM_PACK32 = 51,
	A8B8G8R8_SNORM_PACK32 = 52,
	A8B8G8R8_USCALED_PACK32 = 53,
	A8B8G8R8_SSCALED_PACK32 = 54,
	A8B8G8R8_UINT_PACK32 = 55,
	A8B8G8R8_SINT_PACK32 = 56,
	A8B8G8R8_SRGB_PACK32 = 57,
	R16_UNORM = 70,
	R16_SNORM = 71,
	R16_USCALED = 72,
	R16_SSCALED = 73,
	R16_UINT = 74,
	R16_SINT = 75,
	R16_SFLOAT = 76,
	R16G16_UNORM = 77,
	R16G16_SNORM = 78,
	R16G16_USCALED = 79,
	R16G16_SSCALED = 80,
	R16G16_UINT = 81,
	R16G16_SINT = 82,
	R16G16_SFLOAT = 83,
	R16G16B16_UNORM = 84,
	R16G16B16_SNORM = 85,
	R16G16B16_USCALED = 86,
	R16G16B16_SSCALED = 87,
	R16G16B16_UINT = 88,
	R16G16B16_SINT = 89,
	R16G16B16_SFLOAT = 90,
	R16G16B16A16_UNORM = 91,
	R16G16B16A16_SNORM = 92,
	R16G16B16A16_USCALED = 93,
	R16G16B16A16_SSCALED = 94,
	R16G16B16A16_UINT = 95,
	R16G16B16A16_SINT = 96,
	R16G16B16A16_SFLOAT = 97,
	R32_UINT = 98,
	R32_SINT = 99,
	R32_SFLOAT = 100,
	R32G32_UINT = 101,
	R32G32_SINT = 102,
	R32G32_SFLOAT = 103,
	R32G32B32_UINT = 104,
	R32G32B32_SINT = 105,
	R32G32B32_SFLOAT = 106,
	R32G32B32A32_UINT = 107,
	R32G32B32A32_SINT = 108,
	R32G32B32A32_SFLOAT = 109,
	D16_UNORM = 124,
	D16_UNORM_S8_UINT = 128,
	D24_UNORM_S8_UINT = 129,
	D32_SFLOAT = 126,
	MAX = 0x7FFFFFFF,
};

GL_API size_t get_data_format_size(DataFormat p_format);

enum class CompareOperator : int {
	NEVER = 0,
	LESS = 1,
	EQUAL = 2,
	LESS_OR_EQUAL = 3,
	GREATER = 4,
	NOT_EQUAL = 5,
	GREATER_OR_EQUAL = 6,
	ALWAYS = 7,
};

enum class MemoryAllocationType {
	CPU,
	GPU,
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

enum class ImageLayout : int {
	UNDEFINED = 0,
	GENERAL = 1,
	COLOR_ATTACHMENT_OPTIMAL = 2,
	DEPTH_STENCIL_ATTACHMENT_OPTIMAL = 3,
	DEPTH_STENCIL_READ_ONLY_OPTIMAL = 4,
	SHADER_READ_ONLY_OPTIMAL = 5,
	TRANSFER_SRC_OPTIMAL = 6,
	TRANSFER_DST_OPTIMAL = 7,
	DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL = 1000117000,
	DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL = 1000117001,
	DEPTH_ATTACHMENT_OPTIMAL = 1000241000,
	DEPTH_READ_ONLY_OPTIMAL = 1000241001,
	STENCIL_ATTACHMENT_OPTIMAL = 1000241002,
	STENCIL_READ_ONLY_OPTIMAL = 1000241003,
	READ_ONLY_OPTIMAL = 1000314000,
	ATTACHMENT_OPTIMAL = 1000314001,
	PRESENT_SRC = 1000001002,
};

enum class ImageFiltering : int {
	NEAREST = 0,
	LINEAR = 1,
};

enum class ImageWrappingMode : int {
	REPEAT = 0,
	MIRRORED_REPEAT = 1,
	CLAMP_TO_EDGE = 2,
	CLAMP_TO_BORDER = 3,
	MIRROR_CLAMP_TO_EDGE = 4,
	MAX = 0x7FFFFFFF
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

enum ResolveModeFlags {
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

enum ImageUsageBits {
	IMAGE_USAGE_TRANSFER_SRC_BIT = 0x00000001,
	IMAGE_USAGE_TRANSFER_DST_BIT = 0x00000002,
	IMAGE_USAGE_SAMPLED_BIT = 0x00000004,
	IMAGE_USAGE_STORAGE_BIT = 0x00000008,
	IMAGE_USAGE_COLOR_ATTACHMENT_BIT = 0x00000010,
	IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT = 0x00000020,
};

enum class QueueType {
	GRAPHICS,
	PRESENT,
	TRANSFER,
};

enum class IndexType : size_t {
	UINT16 = 0x00000001,
	UINT32 = 0x00000002,
	MAX = 0xffffffff,
};

enum class PipelineType {
	GRAPHICS,
	COMPUTE,
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

enum ShaderUniformType : size_t {
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

enum class AttachmentLoadOp : int {
	LOAD = 0,
	CLEAR = 1,
	DONT_CARE = 2,
	NONE = 1000400000,
	MAX_ENUM = 0x7FFFFFFF
};

enum class AttachmentStoreOp : int {
	STORE = 0,
	DONT_CARE = 1,
	NONE = 1000301000,
	MAX_ENUM = 0x7FFFFFFF
};

struct RenderPassAttachment {
	DataFormat format;
	AttachmentLoadOp load_op = AttachmentLoadOp::CLEAR;
	AttachmentStoreOp store_op = AttachmentStoreOp::STORE;
	ImageLayout final_layout = ImageLayout::UNDEFINED;
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
	ImageLayout layout = ImageLayout::COLOR_ATTACHMENT_OPTIMAL;
	AttachmentLoadOp load_op = AttachmentLoadOp::DONT_CARE;
	AttachmentStoreOp store_op = AttachmentStoreOp::STORE;
	// load_op must be set to AttachmentLoadOp::CLEAR
	Color clear_color = COLOR_BLACK;

	// For MSAA
	ResolveModeFlags resolve_mode = RESOLVE_MODE_NONE;
	Image resolve_image = GL_NULL_HANDLE;
	ImageLayout resolve_layout = ImageLayout::COLOR_ATTACHMENT_OPTIMAL;
};

enum class RenderPrimitive : int {
	POINT_LIST = 0,
	LINE_LIST = 1,
	LINE_STRIP = 2,
	TRIANGLE_LIST = 3,
	TRIANGLE_STRIP = 4,
	TRIANGLE_FAN = 5,
	LINE_LIST_WITH_ADJACENCY = 6,
	LINE_STRIP_WITH_ADJACENCY = 7,
	TRIANGLE_LIST_WITH_ADJACENCY = 8,
	TRIANGLE_STRIP_WITH_ADJACENCY = 9,
	PATCH_LIST = 10,
};

enum class PolygonCullMode : int {
	DISABLED,
	FRONT,
	BACK,
};

enum class PolygonFrontFace : int {
	CLOCKWISE,
	COUNTER_CLOCKWISE,
};

enum class StencilOperator : int {
	KEEP = 0,
	ZERO = 1,
	REPLACE = 2,
	INCREMENT_AND_CLAMP = 3,
	DECREMENT_AND_CLAMP = 4,
	INVERT = 5,
	INCREMENT_AND_WRAP = 6,
	DECREMENT_AND_WRAP = 7,
};

enum class LogicOperator : int {
	CLEAR = 0,
	AND = 1,
	AND_REVERSE = 2,
	COPY = 3,
	AND_INVERTED = 4,
	NO_OP = 5,
	XOR = 6,
	OR = 7,
	NOR = 8,
	EQUIVALENT = 9,
	INVERT = 10,
	OR_REVERSE = 11,
	COPY_INVERTED = 12,
	OR_INVERTED = 13,
	NAND = 14,
	SET = 15,
};

enum class BlendFactor : int {
	ZERO = 0,
	ONE = 1,
	SRC_COLOR = 2,
	ONE_MINUS_SRC_COLOR = 3,
	DST_COLOR = 4,
	ONE_MINUS_DST_COLOR = 5,
	SRC_ALPHA = 6,
	ONE_MINUS_SRC_ALPHA = 7,
	DST_ALPHA = 8,
	ONE_MINUS_DST_ALPHA = 9,
	CONSTANT_COLOR = 10,
	ONE_MINUS_CONSTANT_COLOR = 11,
	CONSTANT_ALPHA = 12,
	ONE_MINUS_CONSTANT_ALPHA = 13,
	SRC_ALPHA_SATURATE = 14,
	SRC1_COLOR = 15,
	ONE_MINUS_SRC1_COLOR = 16,
	SRC1_ALPHA = 17,
	ONE_MINUS_SRC1_ALPHA = 18,
};

enum class BlendOperation : int {
	ADD = 0,
	SUBTRACT = 1,
	REVERSE_SUBTRACT = 2,
	MIN = 3,
	MAX = 4,
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
	PolygonCullMode cull_mode = PolygonCullMode::DISABLED;
	PolygonFrontFace front_face = PolygonFrontFace::CLOCKWISE;
	bool depth_bias_enabled = false;
	float depth_bias_constant_factor = 0.0f;
	float depth_bias_clamp = 0.0f;
	float depth_bias_slope_factor = 0.0f;
	float line_width = 1.0f;
};

struct PipelineMultisampleState {
	uint32_t sample_count = 1;
	bool enable_sample_shading = false;
	float min_sample_shading = 0.0f;
	std::vector<uint32_t> sample_mask;
	bool enable_alpha_to_coverage = false;
	bool enable_alpha_to_one = false;
};

struct PipelineDepthStencilState {
	bool enable_depth_test = false;
	bool enable_depth_write = false;
	CompareOperator depth_compare_operator = CompareOperator::ALWAYS;
	bool enable_depth_range = false;
	float depth_range_min = 0;
	float depth_range_max = 1.0f;
	bool enable_stencil = false;

	struct StencilOperationState {
		StencilOperator fail = StencilOperator::ZERO;
		StencilOperator pass = StencilOperator::ZERO;
		StencilOperator depth_fail = StencilOperator::ZERO;
		CompareOperator compare = CompareOperator::ALWAYS;
		uint32_t compare_mask = 0;
		uint32_t write_mask = 0;
		uint32_t reference = 0;
	};

	StencilOperationState front_op;
	StencilOperationState back_op;
};

struct PipelineColorBlendState {
	bool enable_logic_op = false;
	LogicOperator logic_op = LogicOperator::CLEAR;

	struct Attachment {
		bool enable_blend = false;
		BlendFactor src_color_blend_factor = BlendFactor::ZERO;
		BlendFactor dst_color_blend_factor = BlendFactor::ZERO;
		BlendOperation color_blend_op = BlendOperation::ADD;
		BlendFactor src_alpha_blend_factor = BlendFactor::ZERO;
		BlendFactor dst_alpha_blend_factor = BlendFactor::ZERO;
		BlendOperation alpha_blend_op = BlendOperation::ADD;
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
			ba.src_color_blend_factor = BlendFactor::SRC_ALPHA;
			ba.dst_color_blend_factor = BlendFactor::ONE_MINUS_SRC_ALPHA;
			ba.src_alpha_blend_factor = BlendFactor::SRC_ALPHA;
			ba.dst_alpha_blend_factor = BlendFactor::ONE_MINUS_SRC_ALPHA;

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

enum class ShaderStage : int {
	VERTEX = 0x00000001,
	TESSELLATION_CONTROL = 0x00000002,
	TESSELLATION_EVALUATION = 0x00000004,
	GEOMETRY = 0x00000008,
	FRAGMENT = 0x00000010,
	COMPUTE = 0x00000020,
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

} //namespace gl