/**
 * @file material.h
 *
 */

#pragma once

#include "glitch/asset/asset.h"
#include "glitch/renderer/texture.h"
#include "glitch/renderer/types.h"

namespace gl {

using ShaderUniformVariable =
		std::variant<int, float, glm::vec2, glm::vec3, glm::vec4, Color, std::weak_ptr<Texture>>;

enum class ShaderUniformVariableType : int {
	INT,
	FLOAT,
	VEC2,
	VEC3,
	VEC4,
	TEXTURE,
};

GL_API size_t uniform_type_std140_alignment(ShaderUniformVariableType p_type);

struct ShaderUniformMetadata {
	std::string name;
	uint32_t binding;
	ShaderUniformVariableType type;
};

struct MaterialDefinition {
	std::string name;

	Shader shader;
	Pipeline pipeline;

	std::vector<ShaderUniformMetadata> uniforms;
};

class GL_API Material {
public:
	GL_REFLECT_ASSET("Material")

	Material(std::shared_ptr<MaterialDefinition> p_definition);
	~Material();

	std::shared_ptr<MaterialDefinition> get_definition() const;
	Pipeline get_pipeline() const;
	Shader get_shader() const;

	UniformSet get_set() const;

	const std::vector<ShaderUniformMetadata>& get_uniforms() const;

	std::optional<ShaderUniformVariable> get_param(const std::string& p_name);
	void set_param(const std::string& p_name, ShaderUniformVariable p_value);

	bool is_dirty() const;

	void upload(); // upload to GPU buffer, descriptor sets, etc.

	// Binds descriptors for set = 0 index = 0
	void bind_uniform_set(CommandBuffer p_cmd);

	// Asset type override
	static std::shared_ptr<Material> create(const std::string& p_def_name);

private:
	std::shared_ptr<MaterialDefinition> definition;

	Buffer material_data_buffer = GL_NULL_HANDLE;
	UniformSet material_set = GL_NULL_HANDLE;

	std::map<std::string, ShaderUniformVariable> params;
	bool dirty = false;
};

static_assert(IsCreatableAsset<Material, std::string>);

class GL_API MaterialSystem {
public:
	MaterialSystem() = delete;

	static void init();
	static void shutdown();

	static void register_definition(const std::string& p_name, MaterialDefinition p_def);

	static std::shared_ptr<Material> create_instance(const std::string& p_def_name);
};

} //namespace gl
