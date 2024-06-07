#include "testbed.h"

#include <core/event/input.h>
#include <renderer/mesh.h>

TestBedApplication::TestBedApplication(const ApplicationCreateInfo& info) :
		Application(info) {}

void TestBedApplication::_on_start() {
	camera = create_ref<PerspectiveCameraNode>();

	get_window()->set_cursor_mode(WindowCursorMode::DISABLED);
	camera_controller.set_camera(camera.get());

	get_renderer()->get_scene_graph().push_node(camera);

	material = Material::create(get_renderer()->get_render_context());

	scene = Mesh::load(get_renderer()->get_render_context(),
			"assets/just_a_girl.glb", material);
	if (scene) {
		for (auto& node : scene->children) {
			node->transform.local_scale = glm::vec3(0.1f);
			node->transform.local_rotation.x = -90.0;
		}
		get_renderer()->get_scene_graph().push_node(scene);
	}
}

void TestBedApplication::_on_update(float dt) {
	camera->aspect_ratio = get_window()->get_aspect_ratio();
	camera_controller.update(dt);
}

void TestBedApplication::_on_destroy() {
	get_renderer()->wait_for_device();

	Material::destroy(get_renderer()->get_render_context(), material);
}
