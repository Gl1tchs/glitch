#include "game.h"

#include <glitch/core/event/input.h>
#include <glitch/renderer/pipeline_builder.h>
#include <glitch/renderer/render_backend.h>
#include <glitch/renderer/renderer.h>
#include <glitch/renderer/shader_library.h>
#include <glitch/scene_graph/gltf_loader.h>
#include <imgui/imgui.h>

Game::Game(const ApplicationCreateInfo& p_info) : Application(p_info) {
	GL_ASSERT(p_info.argc == 2, "A GLTF Model path must be provided.");

	model_path = p_info.argv[1];
}

void Game::_on_start() {
	renderer = create_ref<Renderer>();

	Ref<SceneNode> scene = gltf_loader.load_gltf(model_path);
	scene->transform.scale *= 5.0f;
	scene->transform.rotation.y = 90.0f;

	scene_graph.get_root()->add_child(scene);

	camera_controller.set_camera(&camera, &camera_transform);

	auto [shader, pipeline] =
			PipelineBuilder()
					.add_shader_stage(SHADER_STAGE_VERTEX_BIT,
							ShaderLibrary::get_spirv_data(
									"build/testbed/shaders/"
									"infinite_grid.vert.spv"))
					.add_shader_stage(SHADER_STAGE_FRAGMENT_BIT,
							ShaderLibrary::get_spirv_data(
									"build/testbed/shaders/"
									"infinite_grid.frag.spv"))
					.with_depth_test(
							COMPARE_OP_LESS, false) // without depth write
					.with_blend()
					.build();

	grid_shader = shader;
	grid_pipeline = pipeline;
}

void Game::_on_update(float p_dt) {
	GL_PROFILE_SCOPE;

	static bool mouse_disabled = false;
	if (Input::is_mouse_pressed(MOUSE_BUTTON_RIGHT)) {
		if (!mouse_disabled) {
			get_window()->set_cursor_mode(WINDOW_CURSOR_MODE_DISABLED);
			mouse_disabled = true;
		}

		camera_controller.update(p_dt);
	} else {
		camera_controller.last_mouse_pos.x = Input::get_mouse_position().x;
		camera_controller.last_mouse_pos.y = Input::get_mouse_position().y;
	}

	if (Input::is_mouse_released(MOUSE_BUTTON_RIGHT)) {
		if (mouse_disabled) {
			get_window()->set_cursor_mode(WINDOW_CURSOR_MODE_NORMAL);
			mouse_disabled = false;
		}
	}

	renderer->submit_func([&](CommandBuffer cmd) {
		auto backend = get_rendering_device()->get_backend();

		backend->command_bind_graphics_pipeline(cmd, grid_pipeline);

		GridPushConstants push_constants;
		push_constants.view_proj = camera.get_projection_matrix() *
				camera.get_view_matrix(camera_transform);
		push_constants.camera_pos = camera_transform.position;
		push_constants.grid_size = 100.0f;

		backend->command_push_constants(cmd, grid_shader, 0,
				sizeof(GridPushConstants), &push_constants);

		backend->command_draw(cmd, 6);
	});

	DrawingContext ctx;
	ctx.scene_graph = &scene_graph;
	ctx.camera = camera;
	ctx.camera_transform = camera_transform;

	renderer->submit(ctx);
}

void Game::_on_destroy() {
	auto backend = get_rendering_device()->get_backend();

	backend->device_wait();

	backend->pipeline_free(grid_pipeline);
	backend->shader_free(grid_shader);
}
