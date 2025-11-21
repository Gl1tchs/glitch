/**
 * @file material.h
 *
 */

#pragma once

#include "glitch/asset/asset.h"
#include "glitch/asset/asset_system.h"
#include "glitch/renderer/types.h"

namespace gl {

using ShaderUniformVariable =
		std::variant<int, float, glm::vec2, glm::vec3, glm::vec4, AssetHandle /*  Texture */>;

enum class ShaderUniformVariableType : int {
	INT,
	FLOAT,
	VEC2,
	VEC3,
	VEC4,
	TEXTURE,
};

GL_SERIALIZE_ENUM(ShaderUniformVariableType,
		{
				{ ShaderUniformVariableType::INT, "int" },
				{ ShaderUniformVariableType::FLOAT, "float" },
				{ ShaderUniformVariableType::VEC2, "vec2" },
				{ ShaderUniformVariableType::VEC3, "vec3" },
				{ ShaderUniformVariableType::VEC4, "vec4" },
				{ ShaderUniformVariableType::TEXTURE, "texture" },
		})

struct ShaderUniformMetadata {
	std::string name;
	uint32_t binding;
	ShaderUniformVariableType type;
};

struct MaterialShaderLoadInfo {
	std::string fs_path;
	std::string vs_path;
};

struct MaterialPipelineOptions {
	bool depth_test = true;
	CompareOperator compare_op = CompareOperator::LESS;
	bool depth_write = true;
	bool blend = false;
	RenderPrimitive primitive = RenderPrimitive::LINE_LIST;
};

class GL_API MaterialDefinition {
public:
	GL_REFLECT_ASSET("MaterialDefinition")

	~MaterialDefinition();

	Shader get_shader() const;
	Pipeline get_pipeline() const;

	const MaterialPipelineOptions& get_pipeline_options();

	const std::vector<ShaderUniformMetadata>& get_uniforms();

	static std::shared_ptr<MaterialDefinition> create(
			const std::vector<std::string> p_color_attachment_ids,
			const std::string& p_depth_attachment_id, MaterialShaderLoadInfo p_shader_info,
			std::vector<ShaderUniformMetadata> p_uniforms,
			MaterialPipelineOptions p_pipeline_options = {});

	static bool save(
			const fs::path& p_metadata_path, std::shared_ptr<MaterialDefinition> p_material);
	static std::shared_ptr<MaterialDefinition> load(const fs::path& p_path);

private:
	Shader shader;
	Pipeline pipeline;

	std::vector<std::string> color_attachment_ids;
	std::string depth_attachment_id;
	MaterialShaderLoadInfo shader_info;
	MaterialPipelineOptions pipeline_options;
	std::vector<ShaderUniformMetadata> uniforms;
};

static_assert(IsCreatableAsset<MaterialDefinition, std::vector<std::string>, std::string,
		MaterialShaderLoadInfo, std::vector<ShaderUniformMetadata>, MaterialPipelineOptions>);
static_assert(IsLoadableAsset<MaterialDefinition>);

class GL_API Material {
public:
	GL_REFLECT_ASSET("Material")

	~Material();

	std::shared_ptr<MaterialDefinition> get_definition() const;
	Pipeline get_pipeline() const;
	Shader get_shader() const;

	UniformSet get_set() const;

	const std::vector<ShaderUniformMetadata>& get_uniforms() const;

	std::optional<ShaderUniformVariable> get_param(const std::string& p_name);

	/**
	 * Set material parameter
	 *
	 * @return `false` if parameter not found (uninitialized material)
	 * @return `true` by success
	 */
	bool set_param(const std::string& p_name, ShaderUniformVariable p_value);

	bool is_dirty() const;

	bool upload(); // upload to GPU buffer, descriptor sets, etc.

	// Binds descriptors for set = 0 index = 0
	void bind_uniform_set(CommandBuffer p_cmd);

	// Creates and registers Material to the AssetRegistry
	static std::shared_ptr<Material> create(const std::string& p_def_path);

private:
	std::shared_ptr<MaterialDefinition> definition;

	Buffer material_data_buffer = GL_NULL_HANDLE;
	UniformSet material_set = GL_NULL_HANDLE;

	std::map<std::string, std::pair<ShaderUniformMetadata, ShaderUniformVariable>> params;
	bool dirty = false;
};

static_assert(IsCreatableAsset<Material, std::string>);

} //namespace gl
