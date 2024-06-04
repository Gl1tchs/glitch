#include "testbed.h"

#include <core/event/input.h>

TestBedApplication::TestBedApplication(const ApplicationCreateInfo& info) :
		Application(info) {}

void TestBedApplication::_on_start() {
	camera = create_ref<PerspectiveCameraNode>();

	get_window()->set_cursor_mode(WindowCursorMode::DISABLED);
	camera_controller.set_camera(camera.get());

	get_renderer()->get_scene_graph().push_node(camera);
}

void TestBedApplication::_on_update(float dt) {
	camera->aspect_ratio = get_window()->get_aspect_ratio();
	camera_controller.update(dt);
}

void TestBedApplication::_on_destroy() {}
