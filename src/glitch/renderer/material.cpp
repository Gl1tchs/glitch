#include "glitch/renderer/material.h"

#include "glitch/renderer/render_backend.h"
#include "glitch/renderer/renderer.h"
#include "glitch/renderer/types.h"

MaterialInstance::~MaterialInstance() {
	Ref<RenderBackend> backend = Renderer::get_backend();
	backend->uniform_set_free(uniform_set);
	backend->buffer_free(resources.material_data);
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
	instance->resources = p_resources;

	std::vector<ShaderUniform> uniforms(3);

	uniforms[0].type = UNIFORM_TYPE_UNIFORM_BUFFER;
	uniforms[0].binding = 0;
	uniforms[0].data.push_back(instance->resources.material_data);

	uniforms[1] = instance->resources.albedo_texture->get_uniform(1);
	uniforms[2] = instance->resources.normal_texture->get_uniform(2);

	instance->uniform_set = backend->uniform_set_create(uniforms, shader, 1);

	return instance;
}
