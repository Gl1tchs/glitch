#include "glitch/debug/debug_panel.h"

#include "glitch/core/application.h"
#include "glitch/core/event/input.h"
#include "glitch/core/transform.h"

#include <imgui.h>

DebugPanel::DebugPanel() {}

void DebugPanel::draw() {
	if (Input::is_key_pressed_once(KEY_CODE_F3)) {
		show_panel = !show_panel;
	}

	if (!show_panel) {
		return;
	}

	ImGui::Begin("Stats");
	{
		const ApplicationPerfStats& stats =
				Application::get_instance()->get_perf_stats();

		ImGui::SeparatorText("Application");
		{
			ImGui::Text("Event Loop: %.3f (FPS: %.2f)", stats.delta_time,
					1.0f / std::max(stats.delta_time, 1e-6f));
		}

		ImGui::SeparatorText("RenderDevice");
		{
			ImGui::Text("Draw Calls: %d", stats.renderer_stats.draw_calls);
			ImGui::Text("Index Count: %d", stats.renderer_stats.index_count);
		}
	}
	ImGui::End();

	// static Entity selected_entity = INVALID_ENTITY;
	// ImGui::Begin("Hierarchy");
	// {
	// 	for (EntityId entity_id : p_scene->view()) {
	// 		Entity entity{ entity_id, p_scene };

	// 		if (ImGui::Selectable(std::format("{}", entity.get_name()).c_str(),
	// 					entity == selected_entity)) {
	// 			selected_entity = entity;
	// 		}
	// 	}
	// }
	// ImGui::End();

	// ImGui::Begin("Inspector");
	// if (p_scene->is_valid(selected_entity)) {
	// 	ImGui::Text("Transform");

	// 	Transform* transform = selected_entity.get_transform();

	// 	ImGui::DragFloat3("Position", &transform->local_position.x, 0.1f);
	// 	ImGui::DragFloat3("Rotation", &transform->local_rotation.x, 0.1f);
	// 	ImGui::DragFloat3("Scale", &transform->local_scale.x, 0.1f);

	// 	if (selected_entity.has_component<CameraComponent>()) {
	// 		ImGui::Text("Camera Component");

	// 		CameraComponent* cc =
	// 				selected_entity.get_component<CameraComponent>();

	// 		ImGui::Checkbox("Enabled", &cc->enabled);
	// 		ImGui::DragFloat("Fov", &cc->camera.fov, 0.1f);
	// 		ImGui::DragFloat("Near Clip", &cc->camera.near_clip, 0.1f);
	// 		ImGui::DragFloat("Far Clip", &cc->camera.far_clip, 0.1f);
	// 	}
	// }
	// ImGui::End();
}