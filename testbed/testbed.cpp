#include "testbed.h"
#include "core/window.h"

#include <core/event/input.h>
#include <renderer/mesh.h>
#include <renderer/render_backend.h>
#include <renderer/renderer.h>
#include <renderer/types.h>

#include <imgui.h>

TestBedApplication::TestBedApplication(const ApplicationCreateInfo& p_info) :
		Application(p_info) {}

void TestBedApplication::_on_start() {
	// assign the scene to the renderer
	get_renderer()->set_scene(&scene_graph);

	camera = create_ref<PerspectiveCameraNode>();
	camera->name = "Main Camera";
	scene_graph.push_node(camera);

	camera_controller.set_camera(camera.get());

	material = Material::create();

#if 0
	Ref<Node> sponza =
			Mesh::load("/home/gl1tch/Documents/sponza.glb", material);
	if (sponza) {
		sponza->transform.local_scale *= 0.01f;

		scene_graph.push_node(sponza);
	}
#endif

	Ref<Node> helmet = Mesh::load("assets/DamagedHelmet.glb", material);
	if (helmet) {
		helmet->transform.local_position.y = 1.25f;
		helmet->transform.local_rotation.x = 90.0f;

		scene_graph.push_node(helmet);
	}

	Ref<Node> suzanne = Mesh::load("assets/Suzanne.glb", material);
	if (suzanne) {
		suzanne->transform.local_position.x = 2.5f;
		suzanne->transform.local_position.y = 1.25f;
		suzanne->transform.local_position.z = 4.5f;

		suzanne->transform.local_rotation.y = -30.0f;

		scene_graph.push_node(suzanne);
	}

	grid = create_ref<Grid>();
}

void TestBedApplication::_on_update(float p_dt) {
	camera->aspect_ratio = get_window()->get_aspect_ratio();

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

	static bool imgui_active = true;

	// show/unshow imgui by pressing F1
	static bool f1_key_pressed = false;
	if (!f1_key_pressed && Input::is_key_pressed(KEY_CODE_F1)) {
		imgui_active = !imgui_active;

		f1_key_pressed = true;
	}
	if (f1_key_pressed && Input::is_key_released(KEY_CODE_F1)) {
		f1_key_pressed = false;
	}

	if (imgui_active) {
		get_renderer()->imgui_begin();

		_imgui_render(p_dt);

		get_renderer()->imgui_end();
	}

	if (draw_grid) {
		grid->render(get_renderer(), &scene_graph);
	}
}

void TestBedApplication::_on_destroy() {
	get_renderer()->wait_for_device();

	Material::destroy(material);
}

void TestBedApplication::_imgui_render(float p_dt) {
	// render imgui elements

	_draw_hierarchy();
	_draw_inspector();

	_draw_settings();

	_draw_stats(p_dt);
}

void TestBedApplication::_draw_hierarchy() {
	ImGui::Begin("Scene");
	_draw_node(scene_graph.get_root());
	ImGui::End();
}

void TestBedApplication::_draw_node(const Ref<Node> p_node) {
	ImGui::PushID(p_node->uid.value);

	const bool is_selected = selected_node && selected_node->uid == p_node->uid;

	int flags = ImGuiTreeNodeFlags_DefaultOpen |
			ImGuiTreeNodeFlags_OpenOnArrow |
			ImGuiTreeNodeFlags_OpenOnDoubleClick;
	if (is_selected) {
		flags |= ImGuiTreeNodeFlags_Selected;
	}

	if (p_node->children.size() >= 1) {
		if (ImGui::TreeNodeEx(p_node->name.c_str(), flags)) {
			if (ImGui::IsItemClicked()) {
				selected_node = p_node;
			}

			for (const auto& child : p_node->children) {
				_draw_node(child);
			}

			ImGui::TreePop();
		}
	} else {
		if (ImGui::Selectable(p_node->name.c_str(), is_selected)) {
			selected_node = p_node;
		}
	}

	ImGui::PopID();
}

void TestBedApplication::_draw_inspector() {
	if (selected_node == nullptr) {
		return;
	}

	ImGui::Begin("Inspector");

	ImGui::Text("%s", selected_node->name.c_str());

	ImGui::SeparatorText("Transform");

	ImGui::DragFloat3("Position", &selected_node->transform.local_position.x);
	ImGui::DragFloat3("Rotation", &selected_node->transform.local_rotation.x);
	ImGui::DragFloat3("Scale", &selected_node->transform.local_scale.x);

	ImGui::End();
}

void TestBedApplication::_draw_stats(float dt) {
	const RenderStats& stats = get_renderer()->get_stats();

	ImGui::Begin("Stats");

	ImGui::Text("Delta Time: %.4f", dt);

	ImGui::Text("Draw Calls: %d", stats.draw_calls);
	ImGui::Text("Triangle Count: %d", stats.triangle_count);

	ImGui::End();
}

void TestBedApplication::_draw_settings() {
	ImGui::Begin("Settings");

	ImGui::Checkbox("Draw Grid", &draw_grid);

	ImGui::SliderFloat("Render Scale",
			&get_renderer()->get_settings().render_scale, 0.1f, 1.0f);

	ImGui::End();
}
