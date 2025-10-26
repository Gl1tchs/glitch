#include "editor.h"

#include <glitch/core/event/input.h>
#include <glitch/renderer/pipeline_builder.h>
#include <glitch/renderer/render_backend.h>
#include <glitch/renderer/shader_library.h>
#include <glitch/scene_graph/gltf_loader.h>

#include <imgui.h>
#include <tinyfiledialogs/tinyfiledialogs.h>

EditorApplication::EditorApplication(const ApplicationCreateInfo& p_info) :
		Application(p_info) {}

void EditorApplication::_on_start() {
	SceneRendererSpecification specs = {};
	specs.msaa = 4;

	scene_renderer = create_ref<SceneRenderer>(specs);

	// This must be created after scene renderer for it to initialize materials
	gltf_loader = create_scope<GLTFLoader>();

	camera.transform.position = { -7.98f, 3.48f, -4.18f };
	camera.transform.rotation = { -16.5f, -113.89f, 0.0f };
	camera_controller.set_camera(&camera);

	grid_pass = create_ref<GridPass>();
	get_renderer()->add_pass(grid_pass, -10);
}

void EditorApplication::_on_update(float p_dt) {
	GL_PROFILE_SCOPE;

	grid_pass->set_camera(camera);

	DrawingContext ctx;
	ctx.scene_graph = &scene_graph;
	ctx.camera = camera;
	ctx.settings.resolution_scale = 1.0f;

	scene_renderer->submit(ctx);

	get_renderer()->imgui_begin();
	{
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
		static ImGuiWindowFlags window_flags =
				ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar |
				ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
				ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus |
				ImGuiWindowFlags_NoNavFocus;

		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) {
			window_flags |= ImGuiWindowFlags_NoBackground;
		}

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("GL_DOCK_SPACE", nullptr, window_flags);
		ImGui::PopStyleVar(3);

		// DockSpace
		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		float min_win_size_x = style.WindowMinSize.x;
		style.WindowMinSize.x = 200.0f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
			ImGuiID dockspace_id = ImGui::GetID("GL_DOCK_SPACE");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		style.WindowMinSize.x = min_win_size_x;
		{
			if (ImGui::BeginMainMenuBar()) {
				if (ImGui::BeginMenu("File")) {
					if (ImGui::MenuItem("Load GLTF Model")) {
						constexpr const char* FILTER_PATERNS[2] = { "*.glb",
							"*.gltf" };
						const char* path =
								tinyfd_openFileDialog("Load Model", "",
										sizeof(FILTER_PATERNS) /
												sizeof(FILTER_PATERNS[0]),
										FILTER_PATERNS, "GLTF Files", 0);

						if (path) {
							if (auto scene = gltf_loader->load_gltf(path)) {
								(*scene)->debug_name =
										fs::path(path).filename().string();
								scene_graph.get_root()->add_child(*scene);
							} else {
								GL_LOG_ERROR("{}", scene.get_error());
							}
						}
					}

					ImGui::Separator();

					if (ImGui::MenuItem("Exit")) {
						Application::quit();
					}

					ImGui::EndMenu();
				}

				ImGui::EndMainMenuBar();
			}

			ImGui::Begin("Viewport", nullptr,
					ImGuiWindowFlags_NoScrollbar |
							ImGuiWindowFlags_NoScrollWithMouse);

			// Remove default padding
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });

			// Get final rendered image size (actual texture size)
			const glm::uvec2 image_size =
					get_renderer()->get_final_image_size();
			ImTextureID tex_id = reinterpret_cast<ImTextureID>(
					get_renderer()->get_final_image_descriptor());

			// Get the available region inside the window (without scrollbars or
			// padding)
			ImVec2 avail = ImGui::GetContentRegionAvail();

			// Calculate scale to fit (keep aspect ratio)
			float scale = std::min(avail.x / (float)image_size.x,
					avail.y / (float)image_size.y);

			// Final size after scaling
			ImVec2 final_size = { image_size.x * scale, image_size.y * scale };

			// Center it by adjusting the cursor
			ImVec2 cursor_pos = ImGui::GetCursorPos();
			ImGui::SetCursorPos(
					{ cursor_pos.x + (avail.x - final_size.x) * 0.5f,
							cursor_pos.y + (avail.y - final_size.y) * 0.5f });

			// Draw the image
			ImGui::Image(tex_id, final_size);

			if (ImGui::IsWindowFocused()) {
				static bool mouse_disabled = false;
				if (Input::is_mouse_pressed(MOUSE_BUTTON_RIGHT)) {
					if (!mouse_disabled) {
						get_window()->set_cursor_mode(
								WINDOW_CURSOR_MODE_DISABLED);
						mouse_disabled = true;
					}

					camera_controller.update(p_dt);
				} else {
					camera_controller.last_mouse_pos.x =
							Input::get_mouse_position().x;
					camera_controller.last_mouse_pos.y =
							Input::get_mouse_position().y;
				}

				if (Input::is_mouse_released(MOUSE_BUTTON_RIGHT)) {
					if (mouse_disabled) {
						get_window()->set_cursor_mode(
								WINDOW_CURSOR_MODE_NORMAL);
						mouse_disabled = false;
					}
				}
			}

			ImGui::PopStyleVar();
			ImGui::End();

			ImGui::Begin("Stats");
			{
				const ApplicationPerfStats& stats =
						Application::get_instance()->get_perf_stats();

				ImGui::SeparatorText("Application");
				{
					ImGui::Text("Event Loop: %.3f (FPS: %.2f)",
							stats.delta_time,
							1.0f / std::max(stats.delta_time, 1e-6f));
				}

				ImGui::SeparatorText("Renderer");
				{
					ImGui::Text(
							"Draw Calls: %d", stats.renderer_stats.draw_calls);
					ImGui::Text("Index Count: %d",
							stats.renderer_stats.index_count);
				}
			}
			ImGui::End();

			ImGui::Begin("Hierarchy");
			{
				_traverse_render_node_hierarchy(scene_graph.get_root());
			}
			ImGui::End();

			ImGui::Begin("Inspector");
			if (selected_node) {
				_render_node_properties(selected_node);
			}
			ImGui::End();
		}
		ImGui::End();
	}

	get_renderer()->imgui_end();

	// Delete nodes if any requested
	node_deletion_queue.flush();
}

void EditorApplication::_on_destroy() {}

void EditorApplication::_traverse_render_node_hierarchy(
		const Ref<SceneNode>& p_node) {
	if (!p_node) {
		return;
	}

	ImGui::PushID(p_node->debug_id);

	const std::string label = p_node->debug_name.empty()
			? std::format("{}", p_node->debug_id.value)
			: p_node->debug_name;

	const bool is_selected =
			selected_node && p_node->debug_id == selected_node->debug_id;

	if (p_node->children.empty()) {
		if (ImGui::Selectable(label.c_str(), is_selected)) {
			selected_node = p_node;
		}

		_render_hierarchy_context_menu(p_node);
	} else {
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow |
				ImGuiTreeNodeFlags_OpenOnDoubleClick;

		if (is_selected) {
			flags |= ImGuiTreeNodeFlags_Selected;
		}

		bool node_open = ImGui::TreeNodeEx(label.c_str(), flags);

		if (ImGui::IsItemClicked()) {
			selected_node = p_node;
		}

		_render_hierarchy_context_menu(p_node);

		if (node_open) {
			for (const auto& child : p_node->children) {
				_traverse_render_node_hierarchy(child);
			}
			ImGui::TreePop();
		}
	}

	ImGui::PopID();
}

void EditorApplication::_render_hierarchy_context_menu(
		const Ref<SceneNode>& p_node) {
	if (ImGui::BeginPopupContextItem("HIERARCHY_ITEM_CONTEXT_MENU",
				ImGuiPopupFlags_MouseButtonRight)) {
		if (ImGui::MenuItem("Delete")) {
			node_deletion_queue.push_function([&]() {
				get_render_backend()->device_wait();
				scene_graph.remove_node(p_node->debug_id);
			});
		}
		ImGui::EndPopup();
	}
}

void EditorApplication::_render_node_properties(Ref<SceneNode> p_node) {
	ImGui::Text(
			"ID: %s", std::format("{}", selected_node->debug_id.value).c_str());

	ImGui::SeparatorText("Transform");

	ImGui::DragFloat3("Position", &selected_node->transform.position.x, 0.1f);
	ImGui::DragFloat3("Rotation", &selected_node->transform.rotation.x, 0.1f);
	ImGui::DragFloat3("Scale", &selected_node->transform.scale.x, 0.1f);
}