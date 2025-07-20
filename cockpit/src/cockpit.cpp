#include "cockpit.h"

#include <glitch/core/event/input.h>
#include <glitch/renderer/pipeline_builder.h>
#include <glitch/renderer/render_backend.h>
#include <glitch/renderer/shader_library.h>
#include <glitch/scene_graph/gltf_loader.h>

#include <imgui/imgui.h>

#include "debug_panel.h"

CockpitApplication::CockpitApplication(const ApplicationCreateInfo& p_info) :
		Application(p_info) {
	GL_ASSERT(p_info.argc == 2, "A GLTF Model path must be provided.");

	model_path = p_info.argv[1];
}

void CockpitApplication::_on_start() {
	SceneRendererSpecification specs = {};
	specs.msaa = 4;

	scene_renderer = create_ref<SceneRenderer>(specs);

	// This must be created after scene renderer for it to initialize materials
	gltf_loader = create_scope<GLTFLoader>();

	if (auto res = gltf_loader->load_gltf(model_path); res) {
		Ref<SceneNode> scene = *res;
		scene->transform.scale *= 5.0f;
		scene->transform.rotation.y = 188.0f;

		scene_graph.get_root()->add_child(scene);
	} else {
		GL_LOG_ERROR("{}", res.get_error());
	}

	camera_controller.set_camera(&camera);

	grid_pass = create_ref<GridPass>();
	get_renderer()->add_pass(grid_pass, -10);
}

void CockpitApplication::_on_update(float p_dt) {
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
		ImGui::Begin("EveDockSpace", nullptr, window_flags);
		ImGui::PopStyleVar(3);

		// DockSpace
		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		float min_win_size_x = style.WindowMinSize.x;
		style.WindowMinSize.x = 200.0f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
			ImGuiID dockspace_id = ImGui::GetID("EveDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		style.WindowMinSize.x = min_win_size_x;
		{
			if (ImGui::BeginMainMenuBar()) {
				if (ImGui::BeginMenu("File")) {
					if (ImGui::MenuItem("New")) {
						// handle new
					}
					if (ImGui::MenuItem("Open", "Ctrl+O")) {
						// handle open
					}
					if (ImGui::MenuItem("Save", "Ctrl+S")) {
						// handle save
					}
					ImGui::Separator();
					if (ImGui::MenuItem("Exit", "Alt+F4")) {
						// handle exit
					}
					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Edit")) {
					ImGui::MenuItem("Undo", "Ctrl+Z");
					ImGui::MenuItem("Redo", "Ctrl+Y");
					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("View")) {
					static bool show_console = true;
					ImGui::MenuItem("Show Console", nullptr, &show_console);
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

			DebugPanel::draw(ctx.scene_graph->get_root());
		}
		ImGui::End();
	}
	get_renderer()->imgui_end();
}

void CockpitApplication::_on_destroy() {}
