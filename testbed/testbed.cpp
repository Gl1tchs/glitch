#include "testbed.h"

#include <core/event/input.h>
#include <renderer/mesh.h>
#include <renderer/render_backend.h>
#include <renderer/renderer.h>
#include <renderer/types.h>

#include <imgui.h>

TestBedApplication::TestBedApplication(const ApplicationCreateInfo& info) :
		Application(info) {}

void TestBedApplication::_on_start() {
	camera = create_ref<PerspectiveCameraNode>();

	camera_controller.set_camera(camera.get());

	get_renderer()->get_scene_graph().push_node(camera);

	material = Material::create();

	scene = Mesh::load("assets/DamagedHelmet.glb", material);
	if (scene) {
		for (auto& child : scene->children) {
			child->transform.local_position.y = 0.75f;
			child->transform.local_rotation.x = 90.0f;
		}
		get_renderer()->get_scene_graph().push_node(scene);
	}

	grid = create_ref<Grid>(get_renderer());
}

void TestBedApplication::_on_update(float dt) {
	camera->aspect_ratio = get_window()->get_aspect_ratio();

	static bool mouse_disabled = false;
	if (Input::is_mouse_pressed(MouseCode::RIGHT)) {
		if (!mouse_disabled) {
			get_window()->set_cursor_mode(WindowCursorMode::DISABLED);
			mouse_disabled = true;
		}

		camera_controller.update(dt);
	} else {
		camera_controller.last_mouse_pos.x = Input::get_mouse_position().x;
		camera_controller.last_mouse_pos.y = Input::get_mouse_position().y;
	}

	if (Input::is_mouse_released(MouseCode::RIGHT)) {
		if (mouse_disabled) {
			get_window()->set_cursor_mode(WindowCursorMode::NORMAL);
			mouse_disabled = false;
		}
	}

	static bool imgui_active = true;

	// show/unshow imgui by pressing F1
	static bool f1_key_pressed = false;
	if (!f1_key_pressed && Input::is_key_pressed(KeyCode::F1)) {
		imgui_active = !imgui_active;

		f1_key_pressed = true;
	}
	if (f1_key_pressed && Input::is_key_released(KeyCode::F1)) {
		f1_key_pressed = false;
	}

	if (imgui_active) {
		get_renderer()->imgui_begin();

		_imgui_render();

		get_renderer()->imgui_end();
	}

	grid->render();
}

void TestBedApplication::_on_destroy() {
	get_renderer()->wait_for_device();

	Material::destroy(material);
}

static Ref<Node> s_selected_node = nullptr;

static void _draw_node(const Ref<Node>& node) {
	std::string uid_str = "Node " + std::to_string(node->uid.value);

	bool is_selected = s_selected_node && s_selected_node->uid == node->uid;

	int flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow;
	if (is_selected) {
		flags |= ImGuiTreeNodeFlags_Selected;
	}

	if (node->children.size() >= 1) {
		if (ImGui::TreeNodeEx(uid_str.c_str(), flags)) {
			if (ImGui::IsItemClicked()) {
				s_selected_node = node;
			}

			for (const auto& child : node->children) {
				_draw_node(child);
			}

			ImGui::TreePop();
		}
	} else {
		if (ImGui::Selectable(uid_str.c_str(), is_selected)) {
			s_selected_node = node;
		}
	}
}

static void _draw_hierarchy(const Ref<Node>& root) {
	ImGui::Begin("Scene");
	_draw_node(root);
	ImGui::End();
}

static void _draw_inspector(const Ref<Node> node) {
	if (node == nullptr) {
		return;
	}

	ImGui::Begin("Inspector");

	ImGui::SeparatorText("Transform");

	ImGui::DragFloat3("Position", &node->transform.local_position.x);
	ImGui::DragFloat3("Rotation", &node->transform.local_rotation.x);
	ImGui::DragFloat3("Scale", &node->transform.local_scale.x);

	ImGui::End();
}

void TestBedApplication::_imgui_render() {
	// render imgui elements

	_draw_hierarchy(get_renderer()->get_scene_graph().get_root());
	_draw_inspector(s_selected_node);
}
