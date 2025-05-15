#include "glitch/renderer/material.h"

#include "glitch/renderer/render_backend.h"
#include "glitch/renderer/renderer.h"

MaterialInstance::~MaterialInstance() {
	Ref<RenderBackend> backend = Renderer::get_backend();

	backend->device_wait();

	backend->uniform_set_free(uniform_set);
	backend->buffer_free(material_data_buffer);
}

void MaterialInstance::set_param(
		const std::string& p_name, ShaderUniformVariable p_value) {
	params[p_name] = p_value;
}

void MaterialInstance::set_param(
		const std::string& p_name, Ref<Texture> p_texture) {
	textures[p_name] = p_texture;
}

void MaterialInstance::upload() {
	Ref<RenderBackend> backend = Renderer::get_backend();

	backend->device_wait();

	// free the resources if they already exists
	if (material_data_buffer) {
		backend->buffer_free(material_data_buffer);
	}
	if (uniform_set) {
		backend->uniform_set_free(uniform_set);
	}

	// Calculate required buffer size
	size_t buffer_size = 0;
	std::vector<std::pair<size_t, ShaderUniformMetadata>> write_order;

	for (const auto& [name, param] : params) {
		const auto meta = std::find_if(definition->uniforms.begin(),
				definition->uniforms.end(),
				[&](const ShaderUniformMetadata& u) { return u.name == name; });

		if (meta == definition->uniforms.end()) {
			continue;
		}

		const size_t variant_size = std::visit(
				[](auto&& arg) -> size_t { return sizeof(arg); }, param);

		write_order.push_back({ buffer_size, *meta });

		buffer_size += variant_size;
	}

	// Allocate and write to a contiguous CPU-side buffer
	std::vector<uint8_t> cpu_buffer(buffer_size);
	for (const auto& [offset, meta] : write_order) {
		auto it = params.find(meta.name);
		if (it == params.end()) {
			continue;
		}

		std::visit(
				[&](auto&& arg) {
					using T = std::decay_t<decltype(arg)>;
					std::memcpy(cpu_buffer.data() + offset, &arg, sizeof(T));
				},
				it->second);
	}

	// Create GPU buffer
	material_data_buffer = backend->buffer_create(buffer_size,
			BUFFER_USAGE_UNIFORM_BUFFER_BIT | BUFFER_USAGE_TRANSFER_SRC_BIT,
			MEMORY_ALLOCATION_TYPE_CPU);

	void* gpu_ptr = backend->buffer_map(material_data_buffer);
	std::memcpy(gpu_ptr, cpu_buffer.data(), buffer_size);
	backend->buffer_unmap(material_data_buffer);

	// Bind it to uniform set
	std::vector<ShaderUniform> uniforms;

	ShaderUniform material_data_uniform;
	material_data_uniform.type = UNIFORM_TYPE_UNIFORM_BUFFER;
	material_data_uniform.binding = 0;
	material_data_uniform.data.push_back(material_data_buffer);
	uniforms.push_back(material_data_uniform);

	// Upload textures
	for (const auto& [name, texture] : textures) {
		const auto meta = std::find_if(definition->uniforms.begin(),
				definition->uniforms.end(),
				[&](const ShaderUniformMetadata& u) { return u.name == name; });

		if (meta == definition->uniforms.end()) {
			continue;
		}

		ShaderUniform uniform = texture->get_uniform(meta->binding);
		uniforms.push_back(uniform);
	}

	uniform_set = backend->uniform_set_create(uniforms, definition->shader, 1);
}

MaterialSystem::~MaterialSystem() {
	Ref<RenderBackend> backend = Renderer::get_backend();
	for (auto& [name, definition] : definitions) {
		backend->pipeline_free(definition->pipeline);
		backend->shader_free(definition->shader);
	}
}

void MaterialSystem::register_definition(
		const std::string& p_name, MaterialDefinition p_def) {
	GL_ASSERT(definitions.find(p_name) == definitions.end(),
			"Definition already registered!");

	p_def.name = p_name;
	definitions[p_name] = create_ref<MaterialDefinition>(p_def);
}

Ref<MaterialInstance> MaterialSystem::create_instance(
		const std::string& p_def_name) {
	const auto it = definitions.find(p_def_name);
	if (it == definitions.end()) {
		return nullptr;
	}

	Ref<MaterialInstance> instance = create_ref<MaterialInstance>();
	instance->definition = it->second;

	return instance;
}
