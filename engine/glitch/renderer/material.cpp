#include "glitch/renderer/material.h"

#include "glitch/core/application.h"
#include "glitch/renderer/pipeline_builder.h"
#include "glitch/renderer/render_backend.h"
#include "glitch/renderer/renderer.h"
#include "glitch/renderer/shader_library.h"
#include "glitch/renderer/texture.h"

namespace gl {

MaterialDefinition::~MaterialDefinition() {
	auto backend = Renderer::get_backend();
	backend->shader_free(shader);
	backend->pipeline_free(pipeline);
}

Shader MaterialDefinition::get_shader() const { return shader; }
Pipeline MaterialDefinition::get_pipeline() const { return pipeline; }

const MaterialPipelineOptions& MaterialDefinition::get_pipeline_options() {
	return pipeline_options;
}

const std::vector<ShaderUniformMetadata>& MaterialDefinition::get_uniforms() { return uniforms; }

static std::optional<DataFormat> _get_attachment_by_render_image_id(const std::string& p_id) {
	auto renderer = Application::get()->get_renderer();

	std::optional<Image> image = renderer->get_render_image(p_id);
	if (!image) {
		return std::nullopt;
	}

	return renderer->get_backend()->image_get_format(*image);
}

std::shared_ptr<MaterialDefinition> MaterialDefinition::create(
		const std::vector<std::string> p_color_attachment_ids,
		const std::string& p_depth_attachment_id, const std::string& p_shader_info,
		std::vector<ShaderUniformMetadata> p_uniforms, MaterialPipelineOptions p_pipeline_options) {
	// Spirvv Data should be loaded using ShaderLibrary::get_bundled_spirv
	std::vector<uint32_t> spirv_data;
	if (p_shader_info.starts_with("glitch://")) {
		spirv_data = ShaderLibrary::get_bundled_spirv(
				p_shader_info.substr(sizeof("glitch://") - 1).c_str());
	} else {
		const auto path_abs = AssetSystem::get_absolute_path(p_shader_info);
		if (!path_abs) {
			GL_LOG_ERROR("[MaterialDefinition::create] Path of shader '{}' does not exist.",
					p_shader_info);
			return nullptr;
		}

		spirv_data = ShaderLibrary::get_spirv_data(*path_abs);
	}

	auto builder = PipelineBuilder();

	const auto depth_attachment = _get_attachment_by_render_image_id(p_depth_attachment_id);
	if (!depth_attachment || !is_depth_format(*depth_attachment)) {
		GL_LOG_ERROR("[MaterialDefinition::create] Unable to find depth attachment '{}' in "
					 "renderer context.",
				p_depth_attachment_id);
		return nullptr;
	}

	for (const auto& id : p_color_attachment_ids) {
		const auto color_attachment = _get_attachment_by_render_image_id(id);
		if (!color_attachment) {
			GL_LOG_ERROR("[MaterialDefinition::create] Unable to find color attachment '{}' in "
						 "renderer context.",
					id);
			return nullptr;
		}

		builder.add_color_attachment(*color_attachment);
	}

	builder.set_depth_attachment(depth_attachment)
			.set_shader(spirv_data)
			.with_multisample(Application::get()->get_renderer()->get_msaa_samples(), true)
			.set_render_primitive(p_pipeline_options.primitive);

	if (p_pipeline_options.depth_test) {
		builder.with_depth_test(p_pipeline_options.compare_op, p_pipeline_options.depth_write);
	}

	if (p_pipeline_options.blend) {
		builder.with_blend();
	}

	const auto [shader, pipeline] = builder.build();

	std::shared_ptr<MaterialDefinition> definition = std::make_shared<MaterialDefinition>();
	definition->shader = shader;
	definition->pipeline = pipeline;
	definition->color_attachment_ids = p_color_attachment_ids;
	definition->depth_attachment_id = p_depth_attachment_id;
	definition->shader_info = p_shader_info;
	definition->pipeline_options = p_pipeline_options;
	definition->uniforms = p_uniforms;

	return definition;
}

bool MaterialDefinition::save(
		const fs::path& p_metadata_path, std::shared_ptr<MaterialDefinition> p_definition) {
	if (!p_definition || !p_definition) {
		GL_LOG_ERROR(
				"[MaterialDefinition::save] Unable to save MaterialDefinition to path, invalid "
				"MaterialDefinition object.");
		return false;
	}

	json j;
	j["shader"] = p_definition->shader_info;

	j["color_attachments"] = p_definition->color_attachment_ids;
	j["depth_attachment"] = p_definition->depth_attachment_id;

	j["pipeline"]["depth_test"] = p_definition->pipeline_options.depth_test;
	j["pipeline"]["compare_op"] = p_definition->pipeline_options.compare_op;
	j["pipeline"]["depth_write"] = p_definition->pipeline_options.depth_write;
	j["pipeline"]["blend"] = p_definition->pipeline_options.blend;
	j["pipeline"]["primitive"] = p_definition->pipeline_options.primitive;

	j["uniforms"] = json::array();
	for (const auto& uniform : p_definition->uniforms) {
		json u;
		u["name"] = uniform.name;
		u["binding"] = uniform.binding;
		u["type"] = uniform.type;
		j["uniforms"].push_back(u);
	}

	const auto res = json_save(p_metadata_path.string(), j);
	if (res != JSONLoadError::NONE) {
		if (res == JSONLoadError::FILE_OPEN_ERROR) {
			GL_LOG_ERROR("[MaterialDefinition::save] Unable to save Material metadata to path, "
						 "file open error.");
		} else if (res == JSONLoadError::INVALID_PATH) {
			GL_LOG_ERROR("[MaterialDefinition::save] Unable to save Material metadata to path, "
						 "invalid path.");
		}
		return false;
	}

	return true;
}

std::shared_ptr<MaterialDefinition> MaterialDefinition::load(const fs::path& p_path) {
	if (!fs::exists(p_path)) {
		GL_LOG_ERROR("[MaterialDefinition::load] Unable to load material, given metadata path do "
					 "not exists.");
		return nullptr;
	}

	const auto res = json_load(p_path.string());
	if (!res) {
		GL_LOG_ERROR("[MaterialDefinition::load] Unable to load material, error while parsing "
					 "metadata.");
		return nullptr;
	}

	const json& j = *res;

	if (!j.contains("shader") || !j["shader"].is_object()) {
		GL_LOG_ERROR(
				"[MaterialDefinition::load] Unable to load material, metadata does not contain "
				"shader definition. See 'doc/conventions/material.json'");
		return nullptr;
	}

	const auto shader = j["shader"].get<std::string>();

	if (!j.contains("color_attachments") || !j["color_attachments"].is_array() ||
			j["color_attachments"].empty()) {
		GL_LOG_ERROR("[MaterialDefinition::load] Material definition does not contain any color "
					 "attachments. See 'doc/conventions/material.json'");
		return nullptr;
	}

	if (!j.contains("depth_attachment") || !j["depth_attachment"].is_string()) {
		GL_LOG_ERROR("[MaterialDefinition::load] Material definition does not contain depth "
					 "attachment. See 'doc/conventions/material.json'");
		return nullptr;
	}

	const std::string depth_attachment_id = j["depth_attachment"].get<std::string>();

	const std::vector<std::string> color_attachment_ids =
			j["color_attachments"].get<std::vector<std::string>>();

	MaterialPipelineOptions pipeline_options = {};
	if (j.contains("pipeline")) {
		if (j["pipeline"].contains("depth_test")) {
			j["pipeline"]["depth_test"].get_to(pipeline_options.depth_test);
		}
		if (j["pipeline"].contains("compare_op")) {
			j["pipeline"]["compare_op"].get_to(pipeline_options.compare_op);
		}
		if (j["pipeline"].contains("depth_write")) {
			j["pipeline"]["depth_write"].get_to(pipeline_options.depth_write);
		}
		if (j["pipeline"].contains("blend")) {
			j["pipeline"]["blend"].get_to(pipeline_options.blend);
		}
		if (j["pipeline"].contains("primitive")) {
			j["pipeline"]["primitive"].get_to(pipeline_options.primitive);
		}
	}

	std::vector<ShaderUniformMetadata> uniforms;
	if (j.contains("uniforms") && j["uniforms"].is_array()) {
		for (const auto& uniform : j["uniforms"]) {
			if (!uniform.contains("name") || !uniform.contains("binding") ||
					!uniform.contains("type")) {
				GL_LOG_WARNING("[MaterialDefinition::load] Unable to parse uniform data from path "
							   "'{}'. See 'doc/conventions/material.json'",
						p_path.string());
				continue;
			}

			const auto name = uniform["name"].get<std::string>();
			const uint32_t binding = uniform["binding"].get<uint32_t>();
			const auto type = uniform["type"].get<ShaderUniformVariableType>();

			uniforms.push_back(ShaderUniformMetadata{ name, binding, type });
		}
	}

	return create(color_attachment_ids, depth_attachment_id, shader, uniforms, pipeline_options);
}

Material::~Material() {
	std::shared_ptr<RenderBackend> backend = Renderer::get_backend();

	backend->device_wait();

	backend->uniform_set_free(material_set);
	backend->buffer_free(material_data_buffer);
}

std::shared_ptr<MaterialDefinition> Material::get_definition() const { return definition; }

Pipeline Material::get_pipeline() const { return definition->get_pipeline(); }

Shader Material::get_shader() const { return definition->get_shader(); }

UniformSet Material::get_set() const { return material_set; }

const std::vector<ShaderUniformMetadata>& Material::get_uniforms() const {
	return definition->get_uniforms();
}

std::optional<ShaderUniformVariable> Material::get_param(const std::string& p_name) {
	const auto it = params.find(p_name);
	if (it == params.end()) {
		return {};
	}

	auto& [_, value] = it->second;
	return value;
}

bool Material::set_param(const std::string& p_name, ShaderUniformVariable p_value) {
	const auto it = params.find(p_name);
	if (it == params.end()) {
		return false;
	}

	auto& [_, value] = it->second;
	value = p_value;

	dirty = true;

	return true;
}

bool Material::is_dirty() const { return dirty; }

static AssetHandle _get_default_texture() {
	static AssetHandle s_default_texture = INVALID_ASSET_HANDLE;
	if (!s_default_texture || !AssetSystem::get<Texture>(s_default_texture)) {
		s_default_texture = AssetSystem::register_asset(
				Texture::create(COLOR_WHITE), "mem://texture/material_default");
	}
	return s_default_texture;
}

static constexpr size_t _get_uniform_size(ShaderUniformVariableType type) {
	switch (type) {
		case ShaderUniformVariableType::FLOAT:
			return 4;
		case ShaderUniformVariableType::VEC2:
			return 8;
		case ShaderUniformVariableType::VEC3:
			return 12; // std140 treats vec3 as vec4 usually, but we'll assume packed for now
		case ShaderUniformVariableType::VEC4:
			return 16;
		case ShaderUniformVariableType::INT:
			return 4;
		default:
			return 0;
	}
};

static size_t _uniform_type_std140_alignment(ShaderUniformVariableType p_type) {
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

bool Material::upload() {
	GL_PROFILE_SCOPE;

	std::shared_ptr<RenderBackend> backend = Renderer::get_backend();

	if (!definition) {
		GL_LOG_ERROR("[MaterialInstance::upload] Definition must not be null "
					 "while uploading data.");
		return false;
	}

	// Initialize params
	for (const auto& meta : definition->get_uniforms()) {
		auto it = params.find(meta.name);
		if (it != params.end()) {
			continue;
		}

		// Initialize to default
		ShaderUniformVariable value;
		switch (meta.type) {
			case ShaderUniformVariableType::INT:
				value = int(0);
				break;
			case ShaderUniformVariableType::FLOAT:
				value = float(0.0);
				break;
			case ShaderUniformVariableType::VEC2:
				value = glm::vec2();
				break;
			case ShaderUniformVariableType::VEC3:
				value = glm::vec3();
				break;
			case ShaderUniformVariableType::VEC4:
				value = glm::vec4();
				break;
			case ShaderUniformVariableType::TEXTURE:
				value = _get_default_texture();
				break;
		}

		params[meta.name] = std::make_pair(meta, value);
	}

	// Texture, binding
	std::vector<std::pair<std::shared_ptr<Texture>, int>> textures;

	std::vector<std::byte> cpu_buffer;

	for (const auto& [name, pair] : params) {
		const auto& [meta, value] = pair;

		if (meta.type == ShaderUniformVariableType::TEXTURE) {
			AssetHandle texture_handle = std::get<AssetHandle>(value);

			// Resolve pointer and store
			if (auto texture = AssetSystem::get<Texture>(texture_handle)) {
				textures.push_back(std::make_pair(texture, meta.binding));
			}

			continue; // Textures don't go into the UBO byte array
		}

		// Calculate alignment and padding
		const size_t alignment = _uniform_type_std140_alignment(meta.type);
		const size_t current_size = cpu_buffer.size();
		const size_t aligned_size = align_up(current_size, alignment);

		if (aligned_size > current_size) {
			cpu_buffer.resize(aligned_size, std::byte{ 0 });
		}

		// Determine data size
		// We try to use the variant size if it exists, otherwise fallback to type size
		const auto it = params.find(meta.name);
		const size_t data_size = _get_uniform_size(meta.type);

		// Resize buffer to fit the new data
		const size_t write_offset = cpu_buffer.size();
		cpu_buffer.resize(write_offset + data_size);

		// Write Data
		std::visit(
				[&cpu_buffer, &write_offset](auto&& arg) {
					using T = std::decay_t<decltype(arg)>;
					if constexpr (std::is_same_v<T, AssetHandle>) {
						std::memcpy(cpu_buffer.data() + write_offset, &arg.get_value(), sizeof(T));
					} else {
						std::memcpy(cpu_buffer.data() + write_offset, &arg, sizeof(T));
					}
				},
				value);
	}

	// Create GPU buffer if doesn't exists
	if (!material_data_buffer /* TODO: backend->buffer_size(material_data_buffer) <= cpu_buffer.size() */) {
		// Recreate if too small or non-existent
		// Note: Ideally we might want to keep the old one if it fits, but resizing requires
		// recreation usually
		material_data_buffer = backend->buffer_create(cpu_buffer.size(),
				BUFFER_USAGE_UNIFORM_BUFFER_BIT | BUFFER_USAGE_TRANSFER_SRC_BIT,
				MemoryAllocationType::CPU);
	}

	// TODO: maybe do a persistent buffer to save time
	void* gpu_ptr = backend->buffer_map(material_data_buffer);
	std::memcpy(gpu_ptr, cpu_buffer.data(), cpu_buffer.size());
	backend->buffer_unmap(material_data_buffer);

	// Bind it to uniform set
	std::vector<ShaderUniform> uniforms;
	uniforms.reserve(textures.size() + 1);

	// 0 is the binding of UBO, see: `doc/shader-conventions.md` for more info
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

	if (material_set) {
		backend->uniform_set_free(material_set);
	}

	material_set = backend->uniform_set_create(uniforms, definition->get_shader(), 0);

	dirty = false;

	return true;
}

void Material::bind_uniform_set(CommandBuffer p_cmd) {
	std::shared_ptr<RenderBackend> backend = Renderer::get_backend();
	backend->command_bind_uniform_sets(p_cmd, definition->get_shader(), 0, material_set);
}

std::shared_ptr<Material> Material::create(const std::string& p_def_path) {
	auto definition = AssetSystem::get_by_path<MaterialDefinition>(p_def_path);
	if (!definition) {
		if (const auto res = AssetSystem::load<MaterialDefinition>(p_def_path)) {
			definition = AssetSystem::get<MaterialDefinition>(*res);
		} else {
			GL_LOG_ERROR("[Material::create] Unable to load MaterialDefinition from path '{}'",
					p_def_path);
			return nullptr;
		}
	}

	std::shared_ptr<Material> material = std::make_shared<Material>();
	material->definition = definition;

	// Initialize material
	material->upload();

	return material;
}

} //namespace gl
