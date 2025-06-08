#include "game.h"

#include <glitch/core/event/input.h>
#include <glitch/renderer/gltf_loader.h>
#include <glitch/renderer/renderer.h>
#include <imgui/imgui.h>

Game::Game(const ApplicationCreateInfo& p_info) : Application(p_info) {
	GL_ASSERT(p_info.argc == 2, "A GLTF Model path must be provided.");

	model_path = p_info.argv[1];
}

void Game::_on_start() {
	renderer = create_ref<Renderer>();

	Ref<SceneNode> scene = gltf_loader.load_gltf(model_path);
	scene->transform.scale *= 0.5f;

	scene_graph.get_root()->add_child(scene);

	// Ref<Texture> tex = Texture::create(COLOR_WHITE);
	// Ref<MaterialInstance> material =
	// 		MaterialSystem::create_instance("unlit_standart");
	// material->set_param("base_color", COLOR_RED);
	// material->set_param("u_diffuse_texture", tex);
	// material->upload();

	// Ref<SceneNode> boombox = gltf_loader.load_gltf(
	// 		"C:/Users/gl1tch/Documents/GLTF/BoomBox.glb", material);
	// boombox->transform.scale *= 10.0f;
	// scene_graph.get_root()->add_child(boombox);

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

	DrawingContext ctx;
	ctx.scene_graph = &scene_graph;
	ctx.camera = camera;
	ctx.camera_transform = camera_transform;

	renderer->submit(ctx);
}

void Game::_on_destroy() {}
