#include "glitch/debug/debug_panel.h"

#include "glitch/core/event/input.h"
#include "glitch/core/transform.h"
#include "glitch/scene/components.h"

#include <imgui.h>

DebugPanel::DebugPanel() {}

void DebugPanel::draw(Scene* p_scene) {
	if (Input::is_key_pressed_once(KEY_CODE_F3)) {
		show_panel = !show_panel;
	}

	if (!show_panel) {
		return;
	}

	ImGui::Begin("Debug Panel");

	static Entity selected_entity = INVALID_ENTITY;
	ImGui::Begin("Hierarchy");
	{
		for (auto entity : p_scene->view()) {
			if (ImGui::Selectable(std::format("{}", entity).c_str(),
						entity == selected_entity)) {
				selected_entity = entity;
			}
		}
	}
	ImGui::End();

	ImGui::Begin("Inspector");
	if (p_scene->is_valid(selected_entity)) {
		if (p_scene->has<Transform>(selected_entity)) {
			ImGui::Text("Transform");

			Transform* transform = p_scene->get<Transform>(selected_entity);

			ImGui::DragFloat3("Position", &transform->local_position.x);
			ImGui::DragFloat3("Rotation", &transform->local_rotation.x);
			ImGui::DragFloat3("Scale", &transform->local_scale.x);
		}

		if (p_scene->has<MaterialComponent>(selected_entity)) {
			ImGui::Text("Material Component");

			MaterialComponent* mat =
					p_scene->get<MaterialComponent>(selected_entity);

			ImGui::ColorEdit4("Base Color", &mat->base_color.r);
			ImGui::SliderFloat("Metallic", &mat->metallic, 0.0f, 1.0f);
			ImGui::SliderFloat("Roughness", &mat->roughness, 0.0f, 1.0f);
		}

		if (p_scene->has<CameraComponent>(selected_entity)) {
			ImGui::Text("Camera Component");

			CameraComponent* cc =
					p_scene->get<CameraComponent>(selected_entity);

			ImGui::Checkbox("Enabled", &cc->enabled);
			ImGui::DragFloat("Fov", &cc->camera.fov);
			ImGui::DragFloat("Near Clip", &cc->camera.near_clip);
			ImGui::DragFloat("Far Clip", &cc->camera.far_clip);
		}
	}
	ImGui::End();

	ImGui::End();
}