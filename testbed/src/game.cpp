#include "game.h"

#include <glitch/core/event/input.h>
#include <glitch/scene_graph/gltf_loader.h>
#include <glitch/renderer/renderer.h>
#include <imgui/imgui.h>

Game::Game(const ApplicationCreateInfo& p_info) : Application(p_info) {
	GL_ASSERT(p_info.argc == 2, "A GLTF Model path must be provided.");

	model_path = p_info.argv[1];
}

void Game::_on_start() {
	renderer = create_ref<Renderer>();

	scene_fut = gltf_loader.load_gltf_async(model_path);

	camera_controller.set_camera(&camera, &camera_transform);
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

	if (scene_fut.is_ready()) {
		Ref<SceneNode> scene = scene_fut.get();
		scene->transform.scale *= 0.5f;

		scene_graph.get_root()->add_child(scene);
	}

	DrawingContext ctx;
	ctx.scene_graph = &scene_graph;
	ctx.camera = camera;
	ctx.camera_transform = camera_transform;

	renderer->submit(ctx);
}

void Game::_on_destroy() {}
