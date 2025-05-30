#include "game.h"

#include <glitch/core/event/input.h>
#include <glitch/renderer/mesh_loader.h>
#include <glitch/renderer/renderer.h>
#include <glitch/scene/components.h>
#include <glitch/scene/entity.h>
#include <imgui/imgui.h>

Game::Game(const ApplicationCreateInfo& p_info) : Application(p_info) {}

void Game::_on_start() {
	backend = Renderer::get_backend();

	scene_renderer = create_ref<SceneRenderer>();

	Entity sponza = scene.create("Sponza");
	{
		sponza.get_transform()->local_scale *= 0.01f;

		MeshComponent* mesh_comp = sponza.add_component<MeshComponent>();
		mesh_comp->mesh = scene_renderer->get_mesh_loader().load_mesh(
				"assets/sponza-gltf-pbr/sponza.glb");

		// Load mesh should give an entity tree
	}

	Entity camera = scene.create("Camera");
	{
		camera_transform = camera.get_transform();
		camera_transform->local_position = { 0, 0, 0.5f };

		cc = camera.add_component<CameraComponent>();
	}

	camera_controller.set_camera(&cc->camera, camera_transform);
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

	scene_renderer->render_scene(&scene);
}

void Game::_on_destroy() { backend->device_wait(); }
