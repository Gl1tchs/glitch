/**
 * @file material.h
 *
 */

#pragma once

#include "glitch/renderer/texture.h"
#include "glitch/renderer/types.h"

using ShaderUniformVariable =
		std::variant<int, float, glm::vec2, glm::vec3, glm::vec4, Color>;

enum class ShaderUniformVariableType {
	INT,
	FLOAT,
	VEC2,
	VEC3,
	VEC4,
	TEXTURE,
};

GL_API size_t get_shader_uniform_variable_size(
		ShaderUniformVariableType p_type);

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

struct GL_API MaterialInstance {
	Ref<MaterialDefinition> definition;

	std::map<std::string, ShaderUniformVariable> params;
	std::unordered_map<std::string, Ref<Texture>> textures;

	Buffer material_data_buffer;
	UniformSet uniform_set;

	~MaterialInstance();

	void set_param(const std::string& p_name, ShaderUniformVariable p_value);
	void set_param(const std::string& p_name, Ref<Texture> p_texture);

	void upload(); // upload to GPU buffer, descriptor sets, etc.
};

class MaterialSystem {
public:
	~MaterialSystem();

	void register_definition(
			const std::string& p_name, MaterialDefinition p_def);

	Ref<MaterialInstance> create_instance(const std::string& p_def_name);

private:
	std::unordered_map<std::string, Ref<MaterialDefinition>> definitions;
};