#include "debug_panel.h"

#include <glitch/core/application.h>
#include <glitch/core/transform.h>

#include <imgui/imgui.h>

static SceneNode* s_selected_node = nullptr;

static void _traverse_render_node_hierarchy(const Ref<SceneNode>& p_node) {
	ImGui::PushID(p_node->debug_id);

	const std::string label = p_node->debug_name.empty()
			? std::format("{}", p_node->debug_id.value)
			: p_node->debug_name;

	const bool is_selected =
			s_selected_node && p_node->debug_id == s_selected_node->debug_id;

	if (p_node->children.empty()) {
		if (ImGui::Selectable(label.c_str(), is_selected)) {
			s_selected_node = p_node.get();
		}
	} else {
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow |
				ImGuiTreeNodeFlags_OpenOnDoubleClick;

		if (is_selected) {
			flags |= ImGuiTreeNodeFlags_Selected;
		}

		bool node_open = ImGui::TreeNodeEx(label.c_str(), flags);

		if (ImGui::IsItemClicked()) {
			s_selected_node = p_node.get();
		}

		if (node_open) {
			for (const auto& child : p_node->children) {
				_traverse_render_node_hierarchy(child);
			}
			ImGui::TreePop();
		}
	}

	ImGui::PopID();
}

static void _render_node_properties(SceneNode* p_node) {
	ImGui::Text("ID: %s",
			std::format("{}", s_selected_node->debug_id.value).c_str());

	ImGui::SeparatorText("Transform");

	ImGui::DragFloat3("Position", &s_selected_node->transform.position.x, 0.1f);
	ImGui::DragFloat3("Rotation", &s_selected_node->transform.rotation.x, 0.1f);
	ImGui::DragFloat3("Scale", &s_selected_node->transform.scale.x, 0.1f);
}

void DebugPanel::draw(const Ref<SceneNode>& p_graph_root) {
	ImGui::Begin("Stats");
	{
		const ApplicationPerfStats& stats =
				Application::get_instance()->get_perf_stats();

		ImGui::SeparatorText("Application");
		{
			ImGui::Text("Event Loop: %.3f (FPS: %.2f)", stats.delta_time,
					1.0f / std::max(stats.delta_time, 1e-6f));
		}

		ImGui::SeparatorText("Renderer");
		{
			ImGui::Text("Draw Calls: %d", stats.renderer_stats.draw_calls);
			ImGui::Text("Index Count: %d", stats.renderer_stats.index_count);
		}
	}
	ImGui::End();

	ImGui::Begin("Hierarchy");
	{
		_traverse_render_node_hierarchy(p_graph_root);
	}
	ImGui::End();

	ImGui::Begin("Inspector");
	if (s_selected_node) {
		_render_node_properties(s_selected_node);
	}
	ImGui::End();
}
