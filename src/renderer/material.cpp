#include "glitch/renderer/material.h"

#include "glitch/renderer/render_backend.h"
#include "glitch/renderer/renderer.h"
#include "glitch/renderer/types.h"

#include "shader_bundle.gen.h"

MaterialInstance::~MaterialInstance() {
	Ref<RenderBackend> backend = Renderer::get_backend();
	backend->uniform_set_free(uniform_set);
}

Material::~Material() {
	Ref<RenderBackend> backend = Renderer::get_backend();

	backend->pipeline_free(pipeline);
	backend->shader_free(shader);
}

Ref<MaterialInstance> Material::create_instance(
		const MaterialResources& p_resources) {
	GL_ASSERT(pipeline != GL_NULL_HANDLE);
	GL_ASSERT(shader != GL_NULL_HANDLE);

	Ref<RenderBackend> backend = Renderer::get_backend();

	Ref<MaterialInstance> instance =
			create_ref<MaterialInstance>(MaterialInstance{});
	instance->pipeline = pipeline;
	instance->shader = shader;

	std::vector<ShaderUniform> uniforms(3);

	uniforms[0].type = UNIFORM_TYPE_UNIFORM_BUFFER;
	uniforms[0].binding = 0;
	uniforms[0].data.push_back(p_resources.material_data);

	uniforms[1].type = UNIFORM_TYPE_SAMPLER_WITH_TEXTURE;
	uniforms[1].binding = 1;
	uniforms[1].data.push_back(p_resources.albedo_sampler);
	uniforms[1].data.push_back(p_resources.albedo_image);

	uniforms[2].type = UNIFORM_TYPE_SAMPLER_WITH_TEXTURE;
	uniforms[2].binding = 2;
	uniforms[2].data.push_back(p_resources.normal_sampler);
	uniforms[2].data.push_back(p_resources.normal_image);

	instance->uniform_set = backend->uniform_set_create(uniforms, shader, 1);

	return instance;
}

std::vector<uint32_t> get_bundled_spirv_data(const char* p_path) {
	BundleFileData shader_data = {};
	bool shader_found = false;

	for (int i = 0; i < BUNDLE_FILE_COUNT; i++) {
		BundleFileData data = BUNDLE_FILES[i];
		if (strcmp(data.path, p_path) == 0) {
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
