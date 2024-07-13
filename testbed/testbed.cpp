#include "testbed.h"

#include <core/event/input.h>
#include <renderer/mesh.h>
#include <renderer/render_backend.h>
#include <renderer/renderer.h>
#include <scene/components.h>
#include <scene/scene.h>
#include <scene/view.h>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

TestBedApplication::TestBedApplication(const ApplicationCreateInfo& p_info) :
		Application(p_info) {}

void TestBedApplication::_on_start() {
	// assign the scene to the renderer
	get_renderer()->set_scene(&scene);

	camera_entity = scene.create();
	{
		auto [tag, cc, transform] =
				scene.assign<TagComponent, CameraComponent, Transform>(
						camera_entity);

		tag->name = "Camera";

		cc->is_primary = true;

		camera_controller.set_camera(&cc->camera, transform);
	}

	plane = Model::load_gltf("assets/plane.glb");
	if (plane) {
		Entity plane_entity = scene.create();

		auto [tag, transform, mesh] =
				scene.assign<TagComponent, Transform, MeshRendererComponent>(
						plane_entity);

		tag->name = "Plane";

		transform->local_position = { 3.9f, 2.4f, 0.0f };
		transform->local_rotation = { 7.5f, 7.9f, -0.9f };

		mesh->model = plane;
	}

	gentelman = Model::load_gltf("assets/gentelman.glb");
	if (gentelman) {
		Entity gentelman_entity = scene.create();

		auto [tag, transform, mesh] =
				scene.assign<TagComponent, Transform, MeshRendererComponent>(
						gentelman_entity);

		tag->name = "Gentelman";

		transform->local_position = { -0.6f, 0.8f, 1.9f };
		transform->local_rotation = { 0.0f, 80.0f, 0.0f };
		transform->local_scale = { 1.5f, 1.5f, 1.5f };

		mesh->model = gentelman;
	}

	floor = Model::load_gltf("assets/floor.glb");
	if (floor) {
		Entity floor_entity = scene.create();

		auto [tag, transform, mesh] =
				scene.assign<TagComponent, Transform, MeshRendererComponent>(
						floor_entity);

		tag->name = "Floor";

		transform->local_scale = { 25, 1, 25 };

		mesh->model = floor;
	}

	grid = create_ref<Grid>();
}

void TestBedApplication::_on_update(float p_dt) {
	if (scene.has<CameraComponent>(camera_entity)) {
		scene.get<CameraComponent>(camera_entity)->camera.aspect_ratio =
				get_window()->get_aspect_ratio();
	}

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
		grid->render(get_renderer(), &scene);
	}
}

void TestBedApplication::_on_destroy() {
	get_renderer()->wait_for_device();

	deletion_queue.flush();

	if (plane) {
		Model::destroy(plane);
	}
	if (gentelman) {
		Model::destroy(gentelman);
	}
	if (floor) {
		Model::destroy(floor);
	}
}

void TestBedApplication::_imgui_render(float p_dt) {
	// render imgui elements
	_draw_hierarchy();
	_draw_inspector();
	_draw_settings();
	_draw_stats(p_dt);

	ImGui::SetNextWindowPos(ImVec2(500, 30), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(200, 200), ImGuiCond_Once);
	ImGui::Begin("Image Example", nullptr,
			ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse |
					ImGuiWindowFlags_NoScrollbar);
	{
		Ref<RenderBackend> backend = Renderer::get_backend();

		deletion_queue.flush();

		constexpr uint32_t color = 0xabcdefff;
		Image image = backend->image_create(
				DATA_FORMAT_R8G8B8A8_UNORM, Vec2u{ 1, 1 }, &color);
		deletion_queue.push_function([=]() { backend->image_free(image); });

		ImTextureID imgui_image = backend->imgui_image_upload(
				image, get_renderer()->get_default_sampler());
		deletion_queue.push_function(
				[=]() { backend->imgui_image_free(imgui_image); });

		{
			ImVec2 pos = ImGui::GetCursorScreenPos();
			ImVec2 max_pos = ImGui::GetWindowPos() + ImGui::GetWindowSize();

			ImGui::GetWindowDrawList()->AddImage(
					imgui_image, pos, max_pos - ImVec2(8, 8));
		}
	}
	ImGui::End();
}

constexpr int IMGUI_WINDOW_FLAGS = ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;

void TestBedApplication::_draw_hierarchy() {
	ImGui::Begin("Scene", nullptr, IMGUI_WINDOW_FLAGS);

	for (const Entity entity : scene.view()) {
		_draw_entity(entity);
	}

	ImGui::End();
}

void TestBedApplication::_draw_entity(const Entity p_entity) {
	ImGui::PushID(p_entity);

	const bool is_selected =
			selected_entity != INVALID_ENTITY && selected_entity == p_entity;

	int flags = ImGuiTreeNodeFlags_DefaultOpen |
			ImGuiTreeNodeFlags_OpenOnArrow |
			ImGuiTreeNodeFlags_OpenOnDoubleClick;
	if (is_selected) {
		flags |= ImGuiTreeNodeFlags_Selected;
	}

	const std::string name = scene.has<TagComponent>(p_entity)
			? scene.get<TagComponent>(p_entity)->name
			: std::to_string(p_entity);

	if (ImGui::Selectable(name.c_str(), is_selected)) {
		selected_entity = p_entity;
	}

	ImGui::PopID();
}

void TestBedApplication::_draw_inspector() {
	ImGui::Begin("Inspector", nullptr, IMGUI_WINDOW_FLAGS);

	if (selected_entity == INVALID_ENTITY) {
		ImGui::End();
		return;
	}

	if (scene.has<TagComponent>(selected_entity)) {
		TagComponent* tag = scene.get<TagComponent>(selected_entity);

		ImGui::SeparatorText("Tag");

		ImGui::InputText("Name", &tag->name);
	} else {
		ImGui::Text("%s", std::to_string(selected_entity).c_str());
	}

	if (scene.has<Transform>(selected_entity)) {
		Transform* transform = scene.get<Transform>(selected_entity);

		ImGui::SeparatorText("Transform");

		ImGui::DragFloat3("Position", &transform->local_position.x, 0.1f);
		ImGui::DragFloat3("Rotation", &transform->local_rotation.x, 0.1f);
		ImGui::DragFloat3("Scale", &transform->local_scale.x, 0.1f);
	}

	if (scene.has<CameraComponent>(selected_entity)) {
		CameraComponent* cc = scene.get<CameraComponent>(selected_entity);

		ImGui::SeparatorText("Camera");

		ImGui::DragFloat("FOV", &cc->camera.fov, 0.1f);
		ImGui::DragFloat("Near Clip", &cc->camera.near_clip, 0.1f);
		ImGui::DragFloat("Far Clip", &cc->camera.far_clip, 0.1f);

		ImGui::Checkbox("Primary", &cc->is_primary);
	}

	if (scene.has<MeshRendererComponent>(selected_entity)) {
		MeshRendererComponent* mesh_renderer =
				scene.get<MeshRendererComponent>(selected_entity);

		if (mesh_renderer->model) {
			ImGui::SeparatorText("Mesh Renderer");

			ImGui::TextUnformatted(mesh_renderer->model->name.c_str());
		}
	}

	ImGui::End();
}

void TestBedApplication::_draw_stats(float p_dt) {
	const RenderStats& stats = get_renderer()->get_stats();

	ImGui::Begin("Stats", nullptr, IMGUI_WINDOW_FLAGS);

	ImGui::Text("Delta Time: %.4f", p_dt);

	ImGui::Text("Draw Calls: %d", stats.draw_calls);
	ImGui::Text("Triangle Count: %d", stats.triangle_count);

	ImGui::End();
}

void TestBedApplication::_draw_settings() {
	ImGui::Begin("Settings", nullptr, IMGUI_WINDOW_FLAGS);

	ImGui::Checkbox("Draw Grid", &draw_grid);

	ImGui::SliderFloat("Render Scale",
			&get_renderer()->get_settings().render_scale, 0.1f, 1.0f);

	ImGui::End();
}
