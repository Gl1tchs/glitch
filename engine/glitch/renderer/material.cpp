#include "glitch/renderer/material.h"

#include "glitch/renderer/render_backend.h"
#include "glitch/renderer/renderer.h"

namespace gl {

static std::unordered_map<std::string, Ref<MaterialDefinition>> s_definitions = {};

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

MaterialInstance::MaterialInstance(Ref<MaterialDefinition> p_definition) :
		definition(p_definition) {}

MaterialInstance::~MaterialInstance() {
	Ref<RenderBackend> backend = Renderer::get_backend();

	backend->device_wait();

	backend->uniform_set_free(material_set);
	backend->buffer_free(material_data_buffer);
}

Ref<MaterialDefinition> MaterialInstance::get_definition() const { return definition; }

Pipeline MaterialInstance::get_pipeline() const { return definition->pipeline; }

Shader MaterialInstance::get_shader() const { return definition->shader; }

UniformSet MaterialInstance::get_set() const { return material_set; }

Optional<ShaderUniformVariable> MaterialInstance::get_param(const std::string& p_name) {
	const auto it = params.find(p_name);
	if (it == params.end()) {
		return {};
	}

	return it->second;
}

const std::vector<ShaderUniformMetadata>& MaterialInstance::get_uniforms() const {
	return definition->uniforms;
}

void MaterialInstance::set_param(const std::string& p_name, ShaderUniformVariable p_value) {
	params[p_name] = p_value;
	dirty = true;
}

bool MaterialInstance::is_dirty() const { return dirty; }

void MaterialInstance::upload() {
	GL_PROFILE_SCOPE;

	Ref<RenderBackend> backend = Renderer::get_backend();

	if (!definition) {
		GL_LOG_ERROR("[MaterialInstance::upload] Definition must not be null "
					 "while uploading data.");
		return;
	}

	// Texture, binding
	std::vector<std::pair<Ref<Texture>, int>> textures;

	std::vector<std::pair<size_t, ShaderUniformMetadata>> write_order;
	write_order.reserve(params.size());

	// Calculate required buffer size
	size_t buffer_size = 0;
	for (const auto& [name, param] : params) {
		const auto meta = std::find_if(definition->uniforms.begin(), definition->uniforms.end(),
				[&name](const ShaderUniformMetadata& u) { return u.name == name; });

		if (meta == definition->uniforms.end()) {
			continue;
		}

		if (meta->type == ShaderUniformVariableType::TEXTURE) {
			// TODO: maybe a little overkill
			std::visit(
					[&textures, &meta](const auto& arg) {
						using T = std::decay_t<decltype(arg)>;
						if constexpr (std::is_same_v<T, Ref<Texture>>) {
							textures.push_back(std::make_pair(arg, meta->binding));
						}
					},
					param);
			continue;
		}

		const size_t alignment = uniform_type_std140_alignment(meta->type);
		buffer_size = align_up(buffer_size, alignment);

		const size_t variant_size =
				std::visit([](auto&& arg) -> size_t { return sizeof(arg); }, param);

		write_order.push_back({ buffer_size, *meta });

		buffer_size += variant_size;
	}

	// Allocate and write to a contiguous CPU-side buffer
	std::vector<std::byte> cpu_buffer(buffer_size);
	for (const auto& [offset, meta] : write_order) {
		const auto it = params.find(meta.name);
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
	for (const auto& [texture, binding] : textures) {
		ShaderUniform uniform = texture->get_uniform(binding);
		uniforms.push_back(uniform);
	}

	// free the previous uniform set if already exists
	if (material_set) {
		backend->device_wait();
		backend->uniform_set_free(material_set);
	}

	material_set = backend->uniform_set_create(uniforms, definition->shader, 0);

	dirty = false;
}

void MaterialInstance::bind_uniform_set(CommandBuffer p_cmd) {
	Ref<RenderBackend> backend = Renderer::get_backend();
	backend->command_bind_uniform_sets(p_cmd, definition->shader, 0, material_set);
}

void MaterialSystem::init() { s_definitions.clear(); }

void MaterialSystem::destroy() {
	Ref<RenderBackend> backend = Renderer::get_backend();
	for (auto& [name, definition] : s_definitions) {
		backend->pipeline_free(definition->pipeline);
		backend->shader_free(definition->shader);
	}
}

void MaterialSystem::register_definition(const std::string& p_name, MaterialDefinition p_def) {
	GL_ASSERT(s_definitions.find(p_name) == s_definitions.end(), "Definition already registered!");

	p_def.name = p_name;
	s_definitions[p_name] = create_ref<MaterialDefinition>(p_def);
}

Ref<MaterialInstance> MaterialSystem::create_instance(const std::string& p_def_name) {
	const auto it = s_definitions.find(p_def_name);
	if (it == s_definitions.end()) {
		return nullptr;
	}

	return create_ref<MaterialInstance>(it->second);
}

} //namespace gl