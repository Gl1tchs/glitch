#include "renderer/material.h"

#include "renderer/render_backend.h"
#include "renderer/renderer.h"
#include "renderer/types.h"

#include "shader_bundle.gen.h"

static std::vector<uint32_t> _get_bundled_spirv_data(const char* file_path) {
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

	uint32_t* bundle_data = (uint32_t*)&BUNDLE_DATA[shader_data.start_idx];

	return std::vector<uint32_t>(bundle_data, bundle_data + shader_data.size);
}

Ref<Material> Material::create() {
	Ref<RenderBackend> backend = Renderer::get_backend();

	SpirvData vertex_data = {};
	vertex_data.stage = SHADER_STAGE_VERTEX_BIT;
	vertex_data.byte_code = _get_bundled_spirv_data("mesh.vert.spv");

	SpirvData fragment_data = {};
	fragment_data.stage = SHADER_STAGE_FRAGMENT_BIT;
	fragment_data.byte_code = _get_bundled_spirv_data("mesh.frag.spv");

	std::vector<SpirvData> shader_stages = {
		vertex_data,
		fragment_data,
	};

	Ref<Material> material = create_ref<Material>();

	material->shader = backend->shader_create_from_bytecode(shader_stages);

	PipelineRasterizationState rasterization = {};

	PipelineMultisampleState multisample = {};

	PipelineDepthStencilState depth_stencil_state = {};
	depth_stencil_state.depth_compare_operator = COMPARE_OP_LESS;
	depth_stencil_state.enable_depth_test = true;
	depth_stencil_state.enable_depth_write = true;
	depth_stencil_state.enable_depth_range = true;

	PipelineColorBlendState color_blend_state =
			PipelineColorBlendState::create_disabled();

	RenderingState rendering_state = {};
	rendering_state.color_attachments.push_back(
			Renderer::get_draw_image_format());
	rendering_state.depth_attachment = Renderer::get_depth_image_format();

	material->pipeline = backend->render_pipeline_create(material->shader,
			RENDER_PRIMITIVE_TRIANGLES, rasterization, multisample,
			depth_stencil_state, color_blend_state, 0, rendering_state);

	return material;
}

void Material::destroy(Ref<Material> p_material) {
	if (!p_material) {
		return;
	}

	Ref<RenderBackend> backend = Renderer::get_backend();

	backend->pipeline_free(p_material->pipeline);

	backend->shader_free(p_material->shader);

	for (auto buffer : p_material->allocated_buffers) {
		backend->buffer_free(buffer);
	}
}

Ref<MaterialInstance> Material::create_instance(
		const MaterialResources& resources) {
	Ref<RenderBackend> backend = Renderer::get_backend();

	Ref<MaterialInstance> instance = create_ref<MaterialInstance>();
	instance->pipeline = pipeline;
	instance->shader = shader;

	Buffer constants_buffer = backend->buffer_create(sizeof(MaterialConstants),
			BUFFER_USAGE_TRANSFER_DST_BIT | BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			MEMORY_ALLOCATION_TYPE_CPU);
	allocated_buffers.push_back(constants_buffer);

	MaterialConstants* ptr_constants =
			(MaterialConstants*)backend->buffer_map(constants_buffer);
	*ptr_constants = resources.constants;
	backend->buffer_unmap(constants_buffer);

	std::vector<ShaderUniform> uniforms(4);

	uniforms[0].type = UNIFORM_TYPE_UNIFORM_BUFFER;
	uniforms[0].binding = 0;
	uniforms[0].data.push_back(constants_buffer);

	uniforms[1].type = UNIFORM_TYPE_SAMPLER_WITH_TEXTURE;
	uniforms[1].binding = 1;
	uniforms[1].data.push_back(resources.sampler
					? resources.sampler
					: Renderer::get_default_sampler());
	uniforms[1].data.push_back(resources.diffuse_image
					? resources.diffuse_image
					: Renderer::get_default_image());

	uniforms[2].type = UNIFORM_TYPE_SAMPLER_WITH_TEXTURE;
	uniforms[2].binding = 2;
	uniforms[2].data.push_back(resources.sampler
					? resources.sampler
					: Renderer::get_default_sampler());
	uniforms[2].data.push_back(resources.specular_image
					? resources.specular_image
					: Renderer::get_default_image());

	uniforms[3].type = UNIFORM_TYPE_SAMPLER_WITH_TEXTURE;
	uniforms[3].binding = 3;
	uniforms[3].data.push_back(resources.sampler
					? resources.sampler
					: Renderer::get_default_sampler());
	uniforms[3].data.push_back(resources.normal_image
					? resources.normal_image
					: Renderer::get_default_image());

	instance->uniform_set = backend->uniform_set_create(uniforms, shader, 1);

	return instance;
}
