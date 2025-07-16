#include "glitch/renderer/material_definitions.h"

#include "glitch/renderer/pipeline_builder.h"
#include "glitch/renderer/shader_library.h"
#include "glitch/renderer/types.h"

namespace gl {

MaterialDefinition get_unlit_material_definition(uint32_t p_msaa_samples) {
	auto [shader, pipeline] =
			PipelineBuilder()
					.add_shader_stage(SHADER_STAGE_VERTEX_BIT,
							ShaderLibrary::get_bundled_spirv(
									"pipelines/unlit/mesh.vert.spv"))
					.add_shader_stage(SHADER_STAGE_FRAGMENT_BIT,
							ShaderLibrary::get_bundled_spirv(
									"pipelines/unlit/mesh.frag.spv"))
					.with_depth_test()
					.with_multisample(p_msaa_samples, true)
					.build();

	MaterialDefinition definition;
	definition.shader = shader;
	definition.pipeline = pipeline;
	definition.uniforms = {
		{ "base_color", 0, ShaderUniformVariableType::VEC4 },
		{ "u_diffuse_texture", 1, ShaderUniformVariableType::TEXTURE },
	};

	return definition;
}

MaterialDefinition get_urp_material_definition(uint32_t p_msaa_samples) {
	auto [shader, pipeline] =
			PipelineBuilder()
					.add_shader_stage(SHADER_STAGE_VERTEX_BIT,
							ShaderLibrary::get_bundled_spirv(
									"pipelines/urp/mesh.vert.spv"))
					.add_shader_stage(SHADER_STAGE_FRAGMENT_BIT,
							ShaderLibrary::get_bundled_spirv(
									"pipelines/urp/mesh.frag.spv"))
					.with_depth_test()
					.with_multisample(p_msaa_samples, true)
					.build();

	MaterialDefinition definition;
	definition.shader = shader;
	definition.pipeline = pipeline;
	definition.uniforms = {
		{ "base_color", 0, ShaderUniformVariableType::VEC4 },
		{ "metallic", 0, ShaderUniformVariableType::FLOAT },
		{ "roughness", 0, ShaderUniformVariableType::FLOAT },
		{ "u_diffuse_texture", 1, ShaderUniformVariableType::TEXTURE },
		{ "u_normal_texture", 2, ShaderUniformVariableType::TEXTURE },
		{ "u_metallic_roughness_texture", 3,
				ShaderUniformVariableType::TEXTURE },
		{ "u_ambient_occlusion_texture", 4,
				ShaderUniformVariableType::TEXTURE },
	};

	return definition;
}

} //namespace gl