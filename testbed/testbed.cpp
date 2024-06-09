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

	material = Material::create();

	scene = Mesh::load("assets/DamagedHelmet.glb", material);
	if (scene) {
		for (auto& child : scene->children) {
			child->transform.local_rotation.x = 90.0f;
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

	Material::destroy(material);
}
