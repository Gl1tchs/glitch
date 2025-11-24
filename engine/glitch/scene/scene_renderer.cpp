#include "glitch/scene/scene_renderer.h"

#include "glitch/core/application.h"
#include "glitch/renderer/types.h"

namespace gl {

SceneRenderer::SceneRenderer(const SceneRendererSpecification& p_specs) :
		renderer(Application::get()->get_renderer()), backend(renderer->get_backend()) {
	renderer->set_msaa_samples(p_specs.msaa);

	// Create and initialize clear pass
	// which will define geo_albedo and geo_depth
	clear_pass = std::make_shared<ClearPass>();
	renderer->add_pass(clear_pass, -10);

	// Register material definitions
	{
		const auto unlit_def = MaterialDefinition::create({ "geo_albedo" }, "geo_depth",
				"glitch://pipelines/unlit.slang.spv",
				{
						{
								.name = "base_color",
								.binding = 0,
								.type = ShaderUniformVariableType::VEC4,
						},
						{
								.name = "u_diffuse_texture",
								.binding = 1,
								.type = ShaderUniformVariableType::TEXTURE,
						},
				},
				{
						.depth_test = true,
						.compare_op = CompareOperator::LESS,
						.depth_write = true,
						.blend = false,
						.primitive = RenderPrimitive::TRIANGLE_LIST,
				});
		AssetSystem::register_asset_persistent(unlit_def, DEFINITION_PATH_UNLIT_STANDARD);
	}

	{
		const auto pbr_def = MaterialDefinition::create({ "geo_albedo" }, "geo_depth",
				"glitch://pipelines/pbr.slang.spv",
				{
						{
								.name = "base_color",
								.binding = 0,
								.type = ShaderUniformVariableType::VEC4,
						},
						{
								.name = "metallic",
								.binding = 0,
								.type = ShaderUniformVariableType::FLOAT,
						},
						{
								.name = "roughness",
								.binding = 0,
								.type = ShaderUniformVariableType::FLOAT,
						},
						{
								.name = "u_diffuse_texture",
								.binding = 1,
								.type = ShaderUniformVariableType::TEXTURE,
						},
						{
								.name = "u_normal_texture",
								.binding = 2,
								.type = ShaderUniformVariableType::TEXTURE,
						},
						{
								.name = "u_metallic_roughness_texture",
								.binding = 3,
								.type = ShaderUniformVariableType::TEXTURE,
						},
						{
								.name = "u_ambient_occlusion_texture",
								.binding = 4,
								.type = ShaderUniformVariableType::TEXTURE,
						},
				},
				{
						.depth_test = true,
						.compare_op = CompareOperator::LESS,
						.depth_write = true,
						.blend = false,
						.primitive = RenderPrimitive::TRIANGLE_LIST,
				});
		AssetSystem::register_asset_persistent(pbr_def, DEFINITION_PATH_PBR_STANDARD);
	}

	// Initialize geometry pass
	mesh_pass = std::make_shared<MeshPass>();
	renderer->add_pass(mesh_pass);
}

SceneRenderer::~SceneRenderer() { renderer->wait_for_device(); }

void SceneRenderer::submit(const DrawingContext& p_ctx) {
	GL_PROFILE_SCOPE;

	if (!p_ctx.scene) {
		GL_LOG_WARNING("[SceneRenderer::submit] No Scene assigned to render!");
		return;
	}

	mesh_pass->set_scene(p_ctx.scene);

	renderer->set_render_present_mode(false);
	renderer->set_resolution_scale(p_ctx.settings.resolution_scale);
	renderer->set_vsync(p_ctx.settings.vsync);

	CommandBuffer cmd = renderer->begin_render();
	{
		renderer->execute(cmd);
	}
	renderer->end_render();
}

void SceneRenderer::submit_func(RenderFunc&& p_func) { render_funcs.push_back(p_func); }

} //namespace gl
