#include "glitch/scene/scene_renderer.h"

#include "glitch/core/application.h"
#include "glitch/core/debug/profiling.h"

namespace gl {

SceneRenderer::SceneRenderer(const SceneRendererSpecification& specs) :
		_renderer(Application::get()->get_renderer()), _device(_renderer->get_device()) {
	_renderer->set_msaa_samples(specs.msaa);

	// Create and initialize clear pass
	// which will define geo_albedo and geo_depth
	_clear_pass = std::make_shared<ClearPass>();
	_renderer->add_pass(_clear_pass, -10);

	// Register material definitions
	{
		const auto unlit_def = MaterialDefinition::create({ "geo_albedo" }, "geo_depth",
				{
						.fs_path = "glitch://pipelines/unlit/mesh.frag.spv",
						.vs_path = "glitch://pipelines/unlit/mesh.vert.spv",
				},
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
				{
						.fs_path = "glitch://pipelines/pbr/mesh.frag.spv",
						.vs_path = "glitch://pipelines/pbr/mesh.vert.spv",
				},
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
	_mesh_pass = std::make_shared<MeshPass>();
	_renderer->add_pass(_mesh_pass);
}

SceneRenderer::~SceneRenderer() { _renderer->wait_for_device(); }

void SceneRenderer::submit(const DrawingContext& ctx) {
	GL_PROFILE_SCOPE;

	if (!ctx.scene) {
		GL_LOG_WARNING("[SceneRenderer::submit] No Scene assigned to render!");
		return;
	}

	_mesh_pass->set_scene(ctx.scene);

	_renderer->set_render_present_mode(false);
	_renderer->set_resolution_scale(ctx.settings.resolution_scale);
	_renderer->set_vsync(ctx.settings.vsync);

	CommandBuffer cmd = _renderer->begin_render();
	{
		_renderer->execute(cmd);
	}
	_renderer->end_render();
}

void SceneRenderer::submit_func(RenderFunc&& func) { _render_funcs.push_back(func); }

} //namespace gl
