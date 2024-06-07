#include "renderer/material.h"

#include "platform/vulkan/vk_pipeline.h"
#include "renderer/types.h"

#include "platform/vulkan/vk_shader.h"

#include "shader_bundle.gen.h"

static VectorView<uint32_t> _get_bundled_spirv_data(const char* file_path) {
	BundleFileData shader_data = {};
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
		return {};
	}

	return VectorView<uint32_t>(
			(uint32_t*)&BUNDLE_DATA[shader_data.start_idx], shader_data.size);
}

Ref<Material> Material::create(Context p_context) {
	SpirvData vertex_data = {};
	vertex_data.stage = SHADER_STAGE_VERTEX_BIT;
	vertex_data.byte_code = _get_bundled_spirv_data("triangle.vert.spv");

	SpirvData fragment_data = {};
	fragment_data.stage = SHADER_STAGE_FRAGMENT_BIT;
	fragment_data.byte_code = _get_bundled_spirv_data("triangle.frag.spv");

	std::vector<SpirvData> shader_stages = {
		vertex_data,
		fragment_data,
	};

	Ref<Material> material = create_ref<Material>();

	material->shader =
			vk::shader_create_from_bytecode(p_context, shader_stages);

	PipelineRasterizationState rasterization = {};
	PipelineMultisampleState multisample = {};
	PipelineDepthStencilState depth_state = {};

	PipelineColorBlendState color_blend_state = {};
	color_blend_state.attachments.push_back(
			PipelineColorBlendState::Attachment{});
	color_blend_state.create_disabled();

	RenderingState rendering_state = {};
	rendering_state.color_attachments.push_back(
			DATA_FORMAT_R16G16B16A16_SFLOAT);
	rendering_state.depth_attachment = DATA_FORMAT_D32_SFLOAT;

	material->pipeline = vk::render_pipeline_create(p_context, material->shader,
			RENDER_PRIMITIVE_TRIANGLES, rasterization, multisample, depth_state,
			color_blend_state, 0, rendering_state);

	return material;
}

void Material::destroy(Context p_context, Ref<Material> p_material) {
	vk::pipeline_free(p_context, p_material->pipeline);

	vk::shader_free(p_context, p_material->shader);
}

Ref<MaterialInstance> Material::create_instance(
		Context p_context, const MaterialResources& resources) {}
