#include "glitch/renderer/material.h"

#include "glitch/renderer/render_backend.h"
#include "glitch/renderer/renderer.h"

namespace gl {

static std::unordered_map<std::string, Ref<MaterialDefinition>>
		s_definitions = {};

size_t uniform_type_std140_alignment(ShaderUniformVariableType p_type) {
	switch (p_type) {
		case ShaderUniformVariableType::INT:
			return 4;
		case ShaderUniformVariableType::FLOAT:
			return 4;
		case ShaderUniformVariableType::VEC2:
			return 8;
		case ShaderUniformVariableType::VEC3:
			return 16;
		case ShaderUniformVariableType::VEC4:
			return 16;
		default:
			return 4;
	}
}

MaterialInstance::~MaterialInstance() {
	Ref<RenderBackend> backend = Renderer::get_backend();

	backend->device_wait();

	backend->uniform_set_free(material_set);
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

	if (!definition) {
		GL_LOG_ERROR("MaterialInstance::upload definition must not be null "
					 "while uploading data.");
		return;
	}

	// Calculate required buffer size
	size_t buffer_size = 0;

	std::vector<std::pair<size_t, ShaderUniformMetadata>> write_order;
	write_order.reserve(params.size());

	for (const auto& [name, param] : params) {
		const auto meta = std::find_if(definition->uniforms.begin(),
				definition->uniforms.end(),
				[&name](const ShaderUniformMetadata& u) {
					return u.name == name;
				});

		if (meta == definition->uniforms.end()) {
			continue;
		}

		const size_t alignment = uniform_type_std140_alignment(meta->type);
		buffer_size = align_up(buffer_size, alignment);

		const size_t variant_size = std::visit(
				[](auto&& arg) -> size_t { return sizeof(arg); }, param);

		write_order.push_back({ buffer_size, *meta });

		buffer_size += variant_size;
	}

	// Allocate and write to a contiguous CPU-side buffer
	std::vector<std::byte> cpu_buffer(buffer_size);
	for (const auto& [offset, meta] : write_order) {
		auto it = params.find(meta.name);
		if (it == params.end()) {
			continue;
		}

		std::visit(
				[&cpu_buffer, &offset](auto&& arg) {
					using T = std::decay_t<decltype(arg)>;
					std::memcpy(cpu_buffer.data() + offset, &arg, sizeof(T));
				},
				it->second);
	}

	// Create GPU buffer if doesn't exists
	if (!material_data_buffer) {
		material_data_buffer = backend->buffer_create(buffer_size,
				BUFFER_USAGE_UNIFORM_BUFFER_BIT | BUFFER_USAGE_TRANSFER_SRC_BIT,
				MemoryAllocationType::CPU);
	}

	// TODO: maybe do a persistent buffer to save time
	void* gpu_ptr = backend->buffer_map(material_data_buffer);
	std::memcpy(gpu_ptr, cpu_buffer.data(), buffer_size);
	backend->buffer_unmap(material_data_buffer);

	// Bind it to uniform set
	std::vector<ShaderUniform> uniforms;
	uniforms.reserve(textures.size() + 1);

	ShaderUniform material_data_uniform = {
		.type = UNIFORM_TYPE_UNIFORM_BUFFER,
		.binding = 0,
		.data = { material_data_buffer },
	};
	uniforms.push_back(material_data_uniform);

	// Upload textures
	for (const auto& [name, texture] : textures) {
		const auto meta = std::find_if(definition->uniforms.begin(),
				definition->uniforms.end(),
				[&name](const ShaderUniformMetadata& u) {
					return u.name == name;
				});

		if (meta == definition->uniforms.end()) {
			continue;
		}

		ShaderUniform uniform = texture->get_uniform(meta->binding);
		uniforms.push_back(uniform);
	}

	// free the previous uniform set if already exists
	if (material_set) {
		backend->uniform_set_free(material_set);
	}

	material_set = backend->uniform_set_create(uniforms, definition->shader, 0);
}

void MaterialInstance::bind_uniform_set(CommandBuffer p_cmd) {
	Ref<RenderBackend> backend = Renderer::get_backend();
	backend->command_bind_uniform_sets(
			p_cmd, definition->shader, 0, material_set);
}

void MaterialSystem::init() { s_definitions.clear(); }

void MaterialSystem::destroy() {
	Ref<RenderBackend> backend = Renderer::get_backend();
	for (auto& [name, definition] : s_definitions) {
		backend->pipeline_free(definition->pipeline);
		backend->shader_free(definition->shader);
	}
}

void MaterialSystem::register_definition(
		const std::string& p_name, MaterialDefinition p_def) {
	GL_ASSERT(s_definitions.find(p_name) == s_definitions.end(),
			"Definition already registered!");

	p_def.name = p_name;
	s_definitions[p_name] = create_ref<MaterialDefinition>(p_def);
}

Ref<MaterialInstance> MaterialSystem::create_instance(
		const std::string& p_def_name) {
	const auto it = s_definitions.find(p_def_name);
	if (it == s_definitions.end()) {
		return nullptr;
	}

	Ref<MaterialInstance> instance = create_ref<MaterialInstance>();
	instance->definition = it->second;

	return instance;
}

} //namespace gl