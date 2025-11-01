#include "editor.h"
#include "glitch/scene/components.h"

#include <glitch/core/event/input.h>
#include <glitch/renderer/pipeline_builder.h>
#include <glitch/renderer/render_backend.h>
#include <glitch/renderer/shader_library.h>
#include <glitch/scene/gltf_loader.h>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <tinyfiledialogs/tinyfiledialogs.h>

template <class... Ts> struct overloaded : Ts... {
	using Ts::operator()...;
};

EditorApplication::EditorApplication(const ApplicationCreateInfo& p_info) :
		Application(p_info) {
	renderer_settings.vsync = true;

	scene = create_ref<Scene>();
	runtime_scene = create_ref<Scene>();
}

void EditorApplication::_on_start() {
	ScriptEngine::init();

	SceneRendererSpecification specs = {};
	specs.msaa = 4;

	scene_renderer = create_ref<SceneRenderer>(specs);

	// This must be created after scene renderer for it to initialize materials
	gltf_loader = create_scope<GLTFLoader>();

	Entity camera = scene->create("Camera");
	camera_uid = camera.get_uid();
	camera.get_transform().local_position = { 0.0f, 0.5f, 3.0f };
	camera.get_transform().local_rotation = { -5.0f, 0.0, 0.0f };

	CameraComponent& cc = camera.add_component<CameraComponent>();
	cc.enabled = true;

	camera_controller.set_camera(&cc.camera, &camera.get_transform());

	grid_pass = create_ref<GridPass>();
	get_renderer()->add_pass(grid_pass, -5);

	{
		auto entity = scene->create("Directional Light");

		DirectionalLight& directional_light =
				entity.add_component<DirectionalLight>();

		directional_light.direction = { -1, -1, -1, 0 };
		directional_light.color = COLOR_WHITE;
	}

	{
		auto entity = scene->create("Point Light");
		entity.get_transform().local_position = { 0, 3, 0 };

		PointLight& point_light = entity.add_component<PointLight>();
		point_light.color = COLOR_RED;
		// http://www.ogre3d.org/tikiwiki/tiki-index.php?page=-Point+Light+Attenuation
		point_light.linear = 0.14;
		point_light.quadratic = 0.07;
	}
}

void EditorApplication::_on_update(float p_dt) {
	GL_PROFILE_SCOPE;

	if (is_running) {
		runtime_scene->update(p_dt);
	}

	Entity camera = _get_scene()->find_by_id(camera_uid);
	grid_pass->set_camera(camera.get_component<CameraComponent>()->camera,
			camera.get_transform());

	DrawingContext ctx;
	ctx.scene = _get_scene();
	ctx.settings = renderer_settings;

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
							if (auto model = gltf_loader->load_gltf(
										path, scene)) {
								(*model).set_name(
										fs::path(path).filename().string());
							} else {
								GL_LOG_ERROR("{}", model.get_error());
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

			ImGui::Begin("Settings");
			{
				ImGui::SeparatorText("Editor");

				static bool s_render_grid = true;
				if (ImGui::Checkbox("Render Grid", &s_render_grid)) {
					grid_pass->set_active(s_render_grid);
				}

				ImGui::SeparatorText("Renderer");

				ImGui::DragFloat("Resolution Scale",
						&renderer_settings.resolution_scale, 0.01f, 0.01f, 1.0f,
						"%.2f");
				ImGui::Checkbox("VSync", &renderer_settings.vsync);
			}
			ImGui::End();

			ImGui::Begin("Hierarchy");
			{
				_render_hierarchy();
			}
			ImGui::End();

			ImGui::Begin("Inspector");
			if (selected_entity.is_valid()) {
				_render_inspector(selected_entity);
			}
			ImGui::End();

			ImGui::Begin("Script");

			if (!is_running) {
				if (ImGui::Button("Run Scripts")) {
					// Copy the scene
					scene->copy_to(*runtime_scene);

					selected_entity = Entity(
							(EntityId)selected_entity, runtime_scene.get());

					runtime_scene->start();
					is_running = true;

					Entity camera = _get_scene()->find_by_id(camera_uid);
					camera_controller.set_camera(
							&camera.get_component<CameraComponent>()->camera,
							&camera.get_transform());
				}
			} else {
				if (ImGui::Button("Stop Scripts")) {
					runtime_scene->stop();

					is_running = false;

					selected_entity =
							Entity((EntityId)selected_entity, scene.get());

					Entity camera = _get_scene()->find_by_id(camera_uid);
					camera_controller.set_camera(
							&camera.get_component<CameraComponent>()->camera,
							&camera.get_transform());
				}
			}

			ImGui::End();
		}
		ImGui::End();
	}

	get_renderer()->imgui_end();

	// Delete nodes if any requested
	node_deletion_queue.flush();
}

void EditorApplication::_on_destroy() { ScriptEngine::shutdown(); }

void EditorApplication::_render_hierarchy_entry(Entity p_entity) {
	const std::string label = p_entity.get_name().empty()
			? std::format("Entity {}", p_entity.get_uid().value)
			: p_entity.get_name();

	const bool is_selected = selected_entity.is_valid() &&
			p_entity.get_uid() == selected_entity.get_uid();

	// Get children ahead of time to decide if this is a leaf or a branch
	const auto& children = p_entity.get_children();

	// Leaf Node: The entity has no children.
	if (children.empty()) {
		ImGuiTreeNodeFlags flags =
				ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		if (is_selected) {
			flags |= ImGuiTreeNodeFlags_Selected;
		}

		ImGui::TreeNodeEx((void*)(uint64_t)p_entity.get_uid(), flags, "%s",
				label.c_str());
		if (ImGui::IsItemClicked()) {
			selected_entity = p_entity;
		}

		// Render the context menu for this entity
		_render_hierarchy_context_menu(p_entity);
	}
	// Branch Node: The entity has children.
	else {
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow |
				ImGuiTreeNodeFlags_OpenOnDoubleClick;
		if (is_selected) {
			flags |= ImGuiTreeNodeFlags_Selected;
		}

		// Create a tree node that can be expanded
		bool node_open = ImGui::TreeNodeEx((void*)(uint64_t)p_entity.get_uid(),
				flags, "%s", label.c_str());

		if (ImGui::IsItemClicked()) {
			selected_entity = p_entity;
		}

		// Render the context menu for this entity
		_render_hierarchy_context_menu(p_entity);

		// If the node is open, recursively render all children
		if (node_open) {
			for (const Entity& child : children) {
				_render_hierarchy_entry(child);
			}
			ImGui::TreePop();
		}
	}
}

void EditorApplication::_render_hierarchy() {
	for (Entity entity : _get_scene()->view()) {
		// Only process top-level entities (those without a parent).
		// The recursive function will handle the children.
		if (entity.get_parent()) {
			continue;
		}

		// Start the recursive rendering for each root entity
		_render_hierarchy_entry(entity);
	}
}

void EditorApplication::_render_hierarchy_context_menu(const Entity& p_entity) {
	if (ImGui::BeginPopupContextItem("HIERARCHY_ITEM_CONTEXT_MENU",
				ImGuiPopupFlags_MouseButtonRight)) {
		if (ImGui::MenuItem("Add Child")) {
			static uint32_t s_entity_counter = 0;
			_get_scene()->create(std::format("Entity {}", s_entity_counter++),
					p_entity.get_uid());
		}
		if (ImGui::MenuItem("Delete")) {
			node_deletion_queue.push_function([&]() {
				get_render_backend()->device_wait();
				_get_scene()->destroy(p_entity);
			});
		}
		ImGui::EndPopup();
	}
}

void EditorApplication::_render_inspector(Entity& p_entity) {
	IdComponent* idc = p_entity.get_component<IdComponent>();

	ImGui::Text("ID: %s", std::format("{}", idc->id.value).c_str());
	ImGui::InputText("Name", &idc->tag);

	{
		ImGui::SeparatorText("Transform");
		ImGui::PushID("TRANSFORM_PROPS");

		ImGui::DragFloat3(
				"Position", &p_entity.get_transform().local_position.x, 0.1f);
		ImGui::DragFloat3(
				"Rotation", &p_entity.get_transform().local_rotation.x, 0.1f);
		ImGui::DragFloat3(
				"Scale", &p_entity.get_transform().local_scale.x, 0.1f);

		ImGui::PopID();
	}

	if (p_entity.has_component<MeshComponent>()) {
		ImGui::SeparatorText("Mesh");
		ImGui::PushID("MESH_PROPS");

		MeshComponent* mc = p_entity.get_component<MeshComponent>();
		Ref<Mesh> mesh = MeshSystem::get_mesh(mc->mesh);
		if (mesh) {
			ImGui::Text("Primitives: %zu", mesh->primitives.size());

			ImGui::SeparatorText("Material");

			const Ref<MeshPrimitive> prim = mesh->primitives.front();
			const Ref<MaterialInstance> mat = prim->material;

			for (const ShaderUniformMetadata& uniform : mat->get_uniforms()) {
				ShaderUniformVariable value = *mat->get_param(uniform.name);
				std::visit(
						overloaded{ [&](int& arg) {
									   if (ImGui::InputInt(uniform.name.c_str(),
												   &arg)) {
										   mat->set_param(uniform.name, arg);
									   }
								   },
								[&](float& arg) {
									if (ImGui::InputFloat(
												uniform.name.c_str(), &arg)) {
										mat->set_param(uniform.name, arg);
									}
								},
								[&](glm::vec2& arg) {
									if (ImGui::InputFloat2(
												uniform.name.c_str(), &arg.x)) {
										mat->set_param(uniform.name, arg);
									}
								},
								[&](glm::vec3& arg) {
									if (ImGui::InputFloat3(
												uniform.name.c_str(), &arg.x)) {
										mat->set_param(uniform.name, arg);
									}
								},
								[&](glm::vec4& arg) {
									if (ImGui::InputFloat3(
												uniform.name.c_str(), &arg.x)) {
										mat->set_param(uniform.name, arg);
									}
								},
								[&](Color& arg) {
									if (ImGui::ColorEdit4(
												uniform.name.c_str(), &arg.r)) {
										mat->set_param(uniform.name, arg);
									}
								},
								[](Ref<Texture>& arg) {
									// TODO
								} },
						value);
			}
		}

		ImGui::PopID();
	}

	if (p_entity.has_component<DirectionalLight>()) {
		DirectionalLight* dl = p_entity.get_component<DirectionalLight>();

		ImGui::SeparatorText("Directional Light");
		ImGui::PushID("DIR_LIGHT_PROPS");

		ImGui::DragFloat3("Direction", &dl->direction.x, 0.01f, -1.0f, 1.0f);
		ImGui::ColorEdit3("Color", &dl->color.r);

		ImGui::PopID();
	}

	if (p_entity.has_component<PointLight>()) {
		PointLight* pl = p_entity.get_component<PointLight>();

		ImGui::SeparatorText("Point Light");
		ImGui::PushID("POINT_LIGHT_PROPS");

		ImGui::ColorEdit3("Color", &pl->color.r);
		ImGui::DragFloat("Linear", &pl->linear, 0.01f, 0.0001f, 1.0f);
		ImGui::DragFloat("Quadratic", &pl->quadratic, 0.01f, 0.0001f, 2.0f);

		ImGui::PopID();
	}

	if (p_entity.has_component<ScriptComponent>()) {
		ScriptComponent* sc = p_entity.get_component<ScriptComponent>();

		ImGui::SeparatorText("Script");
		ImGui::PushID("SCRIPT_PROPS");

		ImGui::InputText("Path", &sc->script_path);

		// Unload the script if path changed
		// TODO: more elegant solution
		if (sc->is_loaded) {
			const std::ifstream ifs(sc->script_path);
			if (!ifs.good()) {
				sc->unload();
			}
		}

		// Load the script if it hasn't already
		if (!sc->is_loaded && !sc->script_path.empty()) {
			const std::ifstream ifs(sc->script_path);
			if (ifs.good() && sc->load() != ScriptResult::SUCCESS) {
				sc->script_path = "";
			}
		}

		if (sc->is_loaded) {
			ScriptMetadata metadata = ScriptEngine::get_metadata(sc->script);
			// Only update the metadata if scripts are not running
			if (!is_running) {
				sc->metadata = metadata;
			}

			for (auto& [name, value] : metadata.fields) {
				// skip fields starting with '__'
				if (std::string(name).starts_with("__")) {
					continue;
				}

				std::visit(
						overloaded{ [&](double& arg) {
									   if (ImGui::InputDouble(
												   name.c_str(), &arg)) {
										   ScriptEngine::set_field(sc->script,
												   name.c_str(), arg);
									   }
								   },
								[&](std::string& arg) {
									if (ImGui::InputText(name.c_str(), &arg)) {
										ScriptEngine::set_field(
												sc->script, name.c_str(), arg);
									}
								},
								[&](bool& arg) {
									if (ImGui::Checkbox(name.c_str(), &arg)) {
										ScriptEngine::set_field(
												sc->script, name.c_str(), arg);
									}
								} },
						value);
			}
		}

		ImGui::PopID();
	}

	ImGui::SetCursorPosY(ImGui::GetCursorPos().y +
			ImGui::GetContentRegionAvail().y - ImGui::GetFrameHeight());

	if (ImGui::Button("Add Component", ImVec2(-1, 0))) {
		ImGui::OpenPopup("NODE_ADD_COMPONENT");
	}

	if (ImGui::BeginPopup("NODE_ADD_COMPONENT")) {
		if (!p_entity.has_component<DirectionalLight>() &&
				ImGui::MenuItem("Directional Light")) {
			p_entity.add_component<DirectionalLight>();
		}

		if (!p_entity.has_component<PointLight>() &&
				ImGui::MenuItem("Point Light")) {
			p_entity.add_component<PointLight>();
		}

		if (!p_entity.has_component<ScriptComponent>() &&
				ImGui::MenuItem("Script")) {
			p_entity.add_component<ScriptComponent>();
		}

		ImGui::EndPopup();
	}
}

Ref<Scene> EditorApplication::_get_scene() {
	return is_running ? runtime_scene : scene;
}