#include "glitch/renderer/material_definitions.h"

#include "glitch/renderer/pipeline_builder.h"
#include "glitch/renderer/shader_library.h"
#include "glitch/renderer/types.h"

MaterialDefinition get_unlit_material_definition() {
	auto [shader, pipeline] =
			PipelineBuilder()
					.add_shader_stage(SHADER_STAGE_VERTEX_BIT,
							ShaderLibrary::get_bundled_spirv("mesh.vert.spv"))
					.add_shader_stage(SHADER_STAGE_FRAGMENT_BIT,
							ShaderLibrary::get_bundled_spirv("unlit.frag.spv"))
					.with_depth_test(COMPARE_OP_LESS)
					.build();

	MaterialDefinition definition;
	definition.shader = shader;
	definition.pipeline = pipeline;
	definition.uniforms = {
		{ "base_color", 0, ShaderUniformVariableType::VEC4 },
		{ "metallic", 0, ShaderUniformVariableType::FLOAT },
		{ "roughness", 0, ShaderUniformVariableType::FLOAT },
		{ "u_albedo_texture", 1, ShaderUniformVariableType::TEXTURE },
	};

	return definition;
}

MaterialDefinition get_lit_material_definition() {
	auto [shader, pipeline] =
			PipelineBuilder()
					.add_shader_stage(SHADER_STAGE_VERTEX_BIT,
							ShaderLibrary::get_bundled_spirv("mesh.vert.spv"))
					.add_shader_stage(SHADER_STAGE_FRAGMENT_BIT,
							ShaderLibrary::get_bundled_spirv("lit.frag.spv"))
					.with_depth_test(COMPARE_OP_LESS)
					.build();

	MaterialDefinition definition;
	definition.shader = shader;
	definition.pipeline = pipeline;
	definition.uniforms = {
		{ "base_color", 0, ShaderUniformVariableType::VEC4 },
		{ "metallic", 0, ShaderUniformVariableType::FLOAT },
		{ "roughness", 0, ShaderUniformVariableType::FLOAT },
		{ "u_albedo_texture", 1, ShaderUniformVariableType::TEXTURE },
		{ "u_normal_texture", 1, ShaderUniformVariableType::TEXTURE },
	};

	return definition;
}
