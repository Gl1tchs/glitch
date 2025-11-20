#include "editor.h"

#include "glitch/asset/asset_system.h"
#include "glitch/core/application.h"
#include "glitch/renderer/mesh.h"
#include "glitch/renderer/texture.h"
#include "glitch/scene/components.h"

#include <glitch/core/event/input.h>
#include <glitch/core/templates/variant_helpers.h>
#include <glitch/renderer/pipeline_builder.h>
#include <glitch/renderer/render_backend.h>
#include <glitch/renderer/shader_library.h>
#include <glitch/scene/gltf_loader.h>
#include <glitch/scripting/script.h>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <tinyfiledialogs/tinyfiledialogs.h>

// Helper to draw components cleanly
template <typename T>
void EditorLayer::_draw_component(
		const std::string& name, Entity& entity, std::function<void(T&)> ui_function) {
	if (entity.has_component<T>()) {
		ImGui::PushID((void*)typeid(T).hash_code());
		ImGui::SeparatorText(name.c_str());
		T* component = entity.get_component<T>();
		ui_function(*component);
		ImGui::PopID();
	}
}

void EditorLayer::start() {
	renderer_settings.vsync = true;

	scene = std::make_shared<Scene>();
	runtime_scene = std::make_shared<Scene>();

	SceneRendererSpecification specs = {};
	specs.msaa = 4;
	scene_renderer = std::make_shared<SceneRenderer>(specs);

	// Create initial scene
	Entity camera = scene->create("Camera");
	camera.get_transform().local_position = { 0.0f, 0.5f, 3.0f };
	camera.get_transform().local_rotation = { -5.0f, 0.0, 0.0f };

	CameraComponent* cc = camera.add_component<CameraComponent>();
	cc->enabled = true;

	camera_controller.set_camera(&cc->camera, &camera.get_transform());

	grid_pass = std::make_shared<GridPass>();
	Application::get()->get_renderer()->add_pass(grid_pass, -5);

	{
		auto entity = scene->create("Directional Light");
		DirectionalLight* directional_light = entity.add_component<DirectionalLight>();
		directional_light->direction = { -1, -1, -1, 0 };
		directional_light->color = COLOR_WHITE;
	}
	{
		auto entity = scene->create("Point Light");
		entity.get_transform().local_position = { 0, 3, 0 };
		PointLight* point_light = entity.add_component<PointLight>();
		point_light->color = COLOR_RED;
		point_light->linear = 0.14;
		point_light->quadratic = 0.07;
	}
}

void EditorLayer::update(float p_dt) {
	GL_PROFILE_SCOPE;

	// Logic Update
	if (is_running) {
		runtime_scene->update(p_dt);
	}

	// Sync camera controller
	for (Entity camera : _get_scene()->view<CameraComponent>()) {
		CameraComponent* cc = camera.get_component<CameraComponent>();
		if (cc->enabled) {
			grid_pass->set_camera(cc->camera, camera.get_transform());
			camera_controller.set_camera(&cc->camera, &camera.get_transform());
		}
	}

	// Render Scene
	DrawingContext ctx;
	ctx.scene = _get_scene();
	ctx.settings = renderer_settings;
	scene_renderer->submit(ctx);

	// Render UI
	Application::get()->get_renderer()->imgui_begin();
	{
		_begin_dockspace();
		_render_menubar();
		_render_viewport(p_dt);
		_render_stats();
		_render_settings();

		ImGui::Begin("Hierarchy");
		_render_hierarchy();
		ImGui::End();

		// Hieararchy selects entities and inspector uses it
		// thats why we need to keep this here
		if (selected_entity && selected_entity.get_uid() != selected_id) {
			// Free thumbnails on entity switch
			{
				Renderer::get_backend()->device_wait();
				for (const auto& [_, desc] : thumb_texture_descriptors) {
					Renderer::get_backend()->imgui_image_free(desc);
				}
				thumb_texture_descriptors.clear();
			}
			selected_id = selected_entity.get_uid();
		}

		ImGui::Begin("Inspector");
		if (selected_entity.is_valid()) {
			_render_inspector(selected_entity);
		}
		ImGui::End();

		ImGui::Begin("Asset Registry");
		_render_asset_registry();
		ImGui::End();

		_render_script_panel();
		_end_dockspace();
	}

	Application::get()->get_renderer()->imgui_end();

	frame_deletion_queue.flush();
}

void EditorLayer::destroy() {
	Renderer::get_backend()->device_wait();
	for (const auto& [_, desc] : thumb_texture_descriptors) {
		Renderer::get_backend()->imgui_image_free(desc);
	}
	thumb_texture_descriptors.clear();
}

void EditorLayer::_begin_dockspace() {
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
	static ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

	if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		window_flags |= ImGuiWindowFlags_NoBackground;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("GL_DOCK_SPACE", nullptr, window_flags);
	ImGui::PopStyleVar(3);

	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
		ImGuiID dockspace_id = ImGui::GetID("GL_DOCK_SPACE");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
	}
}

void EditorLayer::_end_dockspace() {
	ImGui::End(); // End GL_DOCK_SPACE
}

void EditorLayer::_render_menubar() {
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (!is_running) {
				if (ImGui::MenuItem("Save Scene")) {
					if (scene_path && fs::exists(*scene_path)) {
						if (!Scene::serialize(scene_path->string(), scene)) {
							GL_LOG_ERROR("Unable to serialize scene");
						}
					} else {
						const char* filter = "*.json";
						if (const char* path = tinyfd_saveFileDialog(
									"Save Scene", "", 1, &filter, "JSON")) {
							if (Scene::serialize(path, scene))
								scene_path = fs::path(path);
						}
					}
				}
				if (ImGui::MenuItem("Load Scene")) {
					const char* filter = "*.json";
					if (const char* path = tinyfd_openFileDialog(
								"Load Scene", "", 1, &filter, "JSON", 0)) {
						if (Scene::deserialize(path, scene)) {
							scene_path = fs::path(path);
						}
					}
				}
			}
			if (ImGui::MenuItem("Load GLTF Model")) {
				const char* filters[] = { "*.glb", "*.gltf" };
				if (const char* path =
								tinyfd_openFileDialog("Load Model", "", 2, filters, "GLTF", 0)) {
					GLTFLoader::load(_get_scene(), path);
				}
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Exit"))
				Application::get()->quit();
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}

void EditorLayer::_render_viewport(float p_dt) {
	// Push zero padding to ensure texture fills the window
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
	ImGui::Begin(
			"Viewport", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

	const ImVec2 viewport_size = ImGui::GetContentRegionAvail();

	// Get Texture Data
	const glm::uvec2 image_size = Application::get()->get_renderer()->get_final_image_size();
	ImTextureID tex_id = reinterpret_cast<ImTextureID>(
			Application::get()->get_renderer()->get_final_image_descriptor());

	// Calculate Aspect Ratio Scale
	const float scale =
			std::min(viewport_size.x / (float)image_size.x, viewport_size.y / (float)image_size.y);
	const ImVec2 final_size = { image_size.x * scale, image_size.y * scale };

	// Center Image
	const ImVec2 cursor_pos = ImGui::GetCursorPos();
	ImGui::SetCursorPos({ cursor_pos.x + (viewport_size.x - final_size.x) * 0.5f,
			cursor_pos.y + (viewport_size.y - final_size.y) * 0.5f });

	ImGui::Image(tex_id, final_size);

	// --- INPUT HANDLING ---

	static bool is_navigating = false;

	// We allow input processing if:
	// 1. We are ALREADY navigating (ignoring whether we are hovered or not)
	// 2. OR We are hovering/focused and want to START navigating
	bool can_handle_input = is_navigating || (ImGui::IsWindowHovered() && ImGui::IsWindowFocused());

	if (can_handle_input) {
		if (Input::is_mouse_pressed(MOUSE_BUTTON_RIGHT)) {
			if (!is_navigating) {
				// Start Navigation
				Application::get()->get_window()->set_cursor_mode(WINDOW_CURSOR_MODE_DISABLED);
				is_navigating = true;
			}

			// Update Camera
			camera_controller.update(p_dt);
		}
	}

	// Handle Release: Check specifically if we WERE navigating and now released
	if (is_navigating && !Input::is_mouse_pressed(MOUSE_BUTTON_RIGHT)) {
		Application::get()->get_window()->set_cursor_mode(WINDOW_CURSOR_MODE_NORMAL);
		is_navigating = false;
	}

	// Prevent "Jumps": Keep syncing last mouse position when we are NOT navigating
	// so the delta is 0 the moment we click.
	if (!is_navigating) {
		camera_controller.last_mouse_pos.x = Input::get_mouse_position().x;
		camera_controller.last_mouse_pos.y = Input::get_mouse_position().y;
	}

	ImGui::End();
	ImGui::PopStyleVar();
}

void EditorLayer::_render_stats() {
	ImGui::Begin("Stats");
	const ApplicationPerfStats& stats = Application::get()->get_perf_stats();
	ImGui::SeparatorText("Application");
	ImGui::Text("DT: %.3f ms (%.1f FPS)", stats.delta_time * 1000.0f,
			1.0f / std::max(stats.delta_time, 1e-6f));

	ImGui::SeparatorText("Renderer");
	ImGui::Text("Draw Calls: %d", stats.renderer_stats.draw_calls);
	ImGui::Text("Indices: %d", stats.renderer_stats.index_count);
	ImGui::End();
}

void EditorLayer::_render_settings() {
	ImGui::Begin("Settings");
	ImGui::SeparatorText("Editor");
	if (ImGui::Checkbox("Render Grid", &show_grid)) {
		grid_pass->set_active(show_grid);
	}
	ImGui::SeparatorText("Renderer");
	ImGui::DragFloat("Res Scale", &renderer_settings.resolution_scale, 0.01f, 0.01f, 1.0f);
	ImGui::Checkbox("VSync", &renderer_settings.vsync);
	ImGui::End();
}

void EditorLayer::_render_inspector(Entity& p_entity) {
	// Calculate Footer Height for "Add Component" button
	float footer_height = ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.y * 2.0f;

	// Begin Child region for scrollable properties, leaving space at bottom
	ImGui::BeginChild("InspectorProperties", ImVec2(0, -footer_height), false);

	IdComponent* idc = p_entity.get_component<IdComponent>();
	ImGui::TextDisabled("UUID: %u", idc->id.value);
	ImGui::InputText("Tag", &idc->tag);

	// Transform
	ImGui::SeparatorText("Transform");
	ImGui::DragFloat3("Pos", &p_entity.get_transform().local_position.x, 0.1f);
	ImGui::DragFloat3("Rot", &p_entity.get_transform().local_rotation.x, 0.1f);
	ImGui::DragFloat3("Scl", &p_entity.get_transform().local_scale.x, 0.1f);

	// Specific Components
	if (p_entity.has_component<GLTFSourceComponent>()) {
		ImGui::SeparatorText("GLTF Info");
		auto* c = p_entity.get_component<GLTFSourceComponent>();
		ImGui::TextWrapped("Path: %s", c->asset_path.c_str());
	}

	_draw_component<CameraComponent>("Camera", p_entity, [](CameraComponent& cc) {
		ImGui::DragFloat("Near", &cc.camera.near_clip);
		ImGui::DragFloat("Far", &cc.camera.far_clip);
		ImGui::DragFloat("FOV", &cc.camera.fov);
		ImGui::Checkbox("Active", &cc.enabled);
	});

	_draw_component<DirectionalLight>("Directional Light", p_entity, [](DirectionalLight& dl) {
		ImGui::DragFloat3("Dir", &dl.direction.x, 0.01f, -1.0f, 1.0f);
		ImGui::ColorEdit3("Color", &dl.color.r);
	});

	_draw_component<PointLight>("Point Light", p_entity, [](PointLight& pl) {
		ImGui::ColorEdit3("Color", &pl.color.r);
		ImGui::DragFloat("Linear", &pl.linear, 0.01f);
		ImGui::DragFloat("Quadratic", &pl.quadratic, 0.01f);
	});

	_draw_component<MeshComponent>("Mesh Component", p_entity, [this](MeshComponent& mc) {
		ImGui::Text("Mesh ID: %u", mc.mesh.value);
		const auto mesh = AssetSystem::get<StaticMesh>(mc.mesh);
		if (!mesh) {
			return;
		}

		ImGui::Text("Index Count: %u", mesh->index_count);
	});

	_draw_component<
			MaterialComponent>("Material Component", p_entity, [this](MaterialComponent& mc) {
		const auto mat = AssetSystem::get<Material>(mc.handle);
		if (!mat) {
			return;
		}

		static std::string s_definition_path = mc.definition_path;
		if (ImGui::InputText(
					"Definition", &s_definition_path, ImGuiInputTextFlags_EnterReturnsTrue)) {
			// Get the definition and recreate the material if definition already exists
			if (const auto definition =
							AssetSystem::get_by_path<MaterialDefinition>(s_definition_path)) {
				if (const auto handle = AssetSystem::create<Material>(s_definition_path)) {
					mc.handle = *handle;
					mc.definition_path = s_definition_path;
				} else {
					GL_LOG_ERROR("[EDITOR] Unable to load MaterialDefinition from path '{}'",
							s_definition_path);
					s_definition_path = mc.definition_path;
				}
			} else {
				// Create new definition otherwise
				if (const auto definition =
								AssetSystem::load<MaterialDefinition>(s_definition_path)) {
					if (const auto handle = AssetSystem::create<Material>(s_definition_path)) {
						mc.handle = *handle;
						mc.definition_path = s_definition_path;
					}
				} else {
					GL_LOG_ERROR("[EDITOR] Unable to load MaterialDefinition from path '{}'",
							s_definition_path);
					s_definition_path = mc.definition_path;
				}
			}
		}

		for (const ShaderUniformMetadata& uniform : mat->get_uniforms()) {
			auto value = mat->get_param(uniform.name);
			if (!value) {
				continue;
			}

			std::visit(
					VariantOverloaded{
							[&](int& arg) {
								if (ImGui::InputInt(uniform.name.c_str(), &arg)) {
									mat->set_param(uniform.name, arg);
								}
							},
							[&](float& arg) {
								if (ImGui::InputFloat(uniform.name.c_str(), &arg)) {
									mat->set_param(uniform.name, arg);
								}
							},
							[&](glm::vec2& arg) {
								if (ImGui::InputFloat2(uniform.name.c_str(), &arg.x)) {
									mat->set_param(uniform.name, arg);
								}
							},
							[&](glm::vec3& arg) {
								if (ImGui::ColorEdit3(uniform.name.c_str(), &arg.x)) {
									mat->set_param(uniform.name, arg);
								}
							},
							[&](glm::vec4& arg) {
								if (ImGui::ColorEdit4(uniform.name.c_str(), &arg.x)) {
									mat->set_param(uniform.name, arg);
								}
							},
							[&](AssetHandle& arg) {
								// Resolve the Asset
								const auto tex = AssetSystem::get<Texture>(arg);
								const auto metadata = AssetSystem::get_metadata<Texture>(arg);

								ImGui::PushID(uniform.name.c_str());

								ImGui::BeginGroup();

								constexpr float thumb_size = 48.0f;
								void* desc_to_render = GL_NULL_HANDLE;

								constexpr const char* k_filter_patterns[5] = { "*.png", "*.jpg",
									"*.jpeg", "*.tga", "*.bmp" };
								auto open_texture_dialog = [&]() {
									const char* path = tinyfd_openFileDialog("Load Texture", "", 5,
											k_filter_patterns, "Image Files", 0);

									if (path) {
										const auto fs_path = fs::path(path);
										const auto metadata_path = fs_path.parent_path() /
												fs_path.filename().replace_extension(std::format(
														"{}.gltex", fs_path.extension().string()));

										// If metadata exists load it from there
										if (fs::exists(metadata_path)) {
											if (auto res = AssetSystem::load<Texture>(
														metadata_path.string())) {
												arg = *res;

												mat->set_param(uniform.name, arg);
											}
										} else {
											// Otherwise create an texture asset and load it.
											const auto texture = Texture::load_from_file(path);
											if (texture) {
												// serialize metadata
												texture->save(metadata_path, texture);

												// TODO! use res:// notation in the future
												arg = AssetSystem::register_asset(
														texture, metadata_path.string());

												mat->set_param(uniform.name, arg);
											}
										}
									}
								};

								if (tex) {
									// Try to find existing descriptor in cache
									const auto it = thumb_texture_descriptors.find(arg.value);

									if (it != thumb_texture_descriptors.end()) {
										desc_to_render = it->second;
									} else {
										// Not found? Upload once and cache it.
										desc_to_render =
												Renderer::get_backend()->imgui_image_upload(
														tex->get_image(), tex->get_sampler());
										thumb_texture_descriptors[arg.value] = desc_to_render;
									}

									// Render Image (Thumbnail)
									ImGui::Image((ImTextureID)desc_to_render,
											ImVec2(thumb_size, thumb_size));

									// Yellow selection border on hover
									if (ImGui::IsItemHovered()) {
										ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(),
												ImGui::GetItemRectMax(),
												IM_COL32(255, 255, 0, 255));

										if (metadata) {
											ImGui::SetTooltip("%s\nDouble-click to "
															  "enlarge\nRight-click to change",
													metadata->path.c_str());
										}
									}

									// Double-click to open popup
									if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
										ImGui::OpenPopup("TEX_PREVIEW_POPUP");
									}

									//  Right Click Context Menu to Load New
									if (ImGui::BeginPopupContextItem("TEX_CONTEXT_MENU")) {
										if (ImGui::MenuItem("Load from file...")) {
											open_texture_dialog();
										}
										if (ImGui::MenuItem("Clear Texture")) {
											arg = AssetHandle(); // Reset to invalid/null handle
											mat->set_param(uniform.name, arg);
										}
										ImGui::EndPopup();
									}

								} else {
									// --- NO TEXTURE ---
									ImGui::PushStyleColor(
											ImGuiCol_Button, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));

									// Click the empty button to open dialog immediately
									if (ImGui::Button(
												"##EmptyTex", ImVec2(thumb_size, thumb_size))) {
										open_texture_dialog();
									}

									ImGui::PopStyleColor();

									if (ImGui::IsItemHovered())
										ImGui::SetTooltip("No Texture Assigned\nClick to Load...");
								}

								// Render uniform name
								ImGui::SameLine();
								float cursor_y = ImGui::GetCursorPosY();
								float center_offset =
										(thumb_size / 2.0f) - (ImGui::GetTextLineHeight() / 2.0f);
								ImGui::SetCursorPosY(cursor_y + center_offset);

								ImGui::Text("%s", uniform.name.c_str());

								ImGui::SetCursorPosY(cursor_y);
								ImGui::EndGroup();

								// --- POPUP LOGIC ---
								if (ImGui::BeginPopup("TEX_PREVIEW_POPUP")) {
									if (tex && desc_to_render) {
										ImGui::Text("Asset: %s",
												(tex && metadata) ? metadata->path.c_str()
																  : "None");
										ImGui::Separator();

										// Reuse the SAME descriptor from the cache!
										// ImGui handles the scaling, so we don't need a separate
										// upload for the big version.
										const float aspect =
												(float)tex->get_size().x / (float)tex->get_size().y;
										const float width = 300.0f;

										ImGui::Image((ImTextureID)desc_to_render,
												ImVec2(width, width / aspect));
									}
									ImGui::EndPopup();
								}

								ImGui::PopID();
							},
					},
					*value);
		}
	});

	// SCRIPTING
	_draw_component<Script>("Script", p_entity, [this](Script& sc) {
		ImGui::InputText("Script Path", &sc.script_path);

		// Logic to load/unload script
		if (sc.is_loaded) {
			if (!std::ifstream(sc.script_path).good())
				sc.unload();
		} else if (!sc.script_path.empty() && std::ifstream(sc.script_path).good()) {
			sc.load();
		}

		if (sc.is_loaded && !is_running) {
			sc.metadata = ScriptEngine::get_metadata(sc.script);
		}

		// Render script fields
		if (sc.is_loaded) {
			for (auto& [name, value] : sc.metadata->fields) {
				if (name.starts_with("__")) {
					continue;
				}

				std::visit(VariantOverloaded{
								   [&](double& v) {
									   ImGui::InputDouble(name.c_str(), &v) &&
											   ScriptEngine::set_field(sc.script, name.c_str(), v);
								   },
								   [&](std::string& v) {
									   ImGui::InputText(name.c_str(), &v) &&
											   ScriptEngine::set_field(sc.script, name.c_str(), v);
								   },
								   [&](bool& v) {
									   ImGui::Checkbox(name.c_str(), &v) &&
											   ScriptEngine::set_field(sc.script, name.c_str(), v);
								   },
						   },
						value);
			}
		}
	});

	ImGui::EndChild(); // End Properties Area

	ImGui::Separator();
	if (ImGui::Button("Add Component", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
		ImGui::OpenPopup("ADD_COMPONENT_POPUP");
	}

	if (ImGui::BeginPopup("ADD_COMPONENT_POPUP")) {
		if (!p_entity.has_component<DirectionalLight>()) {
			if (ImGui::MenuItem("Directional Light"))
				p_entity.add_component<DirectionalLight>();
		}
		if (!p_entity.has_component<PointLight>()) {
			if (ImGui::MenuItem("Point Light"))
				p_entity.add_component<PointLight>();
		}
		if (!p_entity.has_component<Script>()) {
			if (ImGui::MenuItem("Script"))
				p_entity.add_component<Script>();
		}
		ImGui::EndPopup();
	}
}

void EditorLayer::_render_script_panel() {
	ImGui::Begin("Script");
	if (!is_running) {
		if (ImGui::Button("Run Scripts", ImVec2(-1, 0))) {
			scene->copy_to(*runtime_scene);
			selected_entity = Entity((EntityId)selected_entity, runtime_scene.get());
			runtime_scene->start();
			is_running = true;
		}
	} else {
		if (ImGui::Button("Stop Scripts", ImVec2(-1, 0))) {
			runtime_scene->stop();
			is_running = false;
			selected_entity = Entity((EntityId)selected_entity, scene.get());
		}
	}
	ImGui::End();
}

void EditorLayer::_render_hierarchy() {
	for (Entity entity : _get_scene()->view()) {
		if (entity.is_valid() && !entity.get_parent()) {
			ImGui::PushID(entity.get_uid().value);
			_render_hierarchy_entry(entity);
			ImGui::PopID();
		}
	}
}

void EditorLayer::_render_hierarchy_entry(Entity p_entity) {
	if (!p_entity.is_valid()) {
		return;
	}

	const std::string label = p_entity.get_name().empty() ? "Entity" : p_entity.get_name();
	const bool is_selected =
			selected_entity.is_valid() && p_entity.get_uid() == selected_entity.get_uid();
	const auto children = p_entity.get_children();

	ImGuiTreeNodeFlags flags =
			(children.empty() ? ImGuiTreeNodeFlags_Leaf : ImGuiTreeNodeFlags_OpenOnArrow) |
			ImGuiTreeNodeFlags_OpenOnDoubleClick | (is_selected ? ImGuiTreeNodeFlags_Selected : 0);

	const bool node_open =
			ImGui::TreeNodeEx((void*)(uint64_t)p_entity.get_uid(), flags, "%s", label.c_str());

	if (ImGui::IsItemClicked()) {
		selected_entity = p_entity;
	}

	_render_hierarchy_context_menu(p_entity);

	if (node_open) {
		for (const Entity& child : children) {
			ImGui::PushID(child.get_uid().value);
			_render_hierarchy_entry(child);
			ImGui::PopID();
		}
		ImGui::TreePop();
	}
}

void EditorLayer::_render_hierarchy_context_menu(Entity p_entity) {
	if (ImGui::BeginPopupContextItem()) {
		if (ImGui::MenuItem("Add Child")) {
			_get_scene()->create("New Entity", p_entity);
		}
		if (ImGui::MenuItem("Delete")) {
			frame_deletion_queue.push_function([this, p_entity]() {
				Renderer::get_backend()->device_wait();
				if (selected_entity && selected_entity.get_uid() == p_entity.get_uid())
					selected_entity = INVALID_ENTITY;
				_get_scene()->destroy(p_entity);
				AssetSystem::collect_garbage();
			});
		}
		ImGui::EndPopup();
	}
}

void EditorLayer::_render_asset_registry() {
	const float button_height = ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.y * 2.0f;

	ImGui::BeginChild("AssetRegistryChild", ImVec2(-1, -button_height));

	if (ImGui::BeginTable("AssetRegistryTable", 3,
				ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollY |
						ImGuiTableFlags_ScrollX)) {
		ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed);
		ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed);
		ImGui::TableSetupColumn("Path",
				ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoSort |
						ImGuiTableColumnFlags_NoResize);
		ImGui::TableHeadersRow();

		for (const auto& [handle, metadata] : AssetSystem::get_asset_metadata()) {
			ImGui::TableNextRow();

			ImGui::TableNextColumn();
			ImGui::Text("%s", metadata.type_name);

			ImGui::TableNextColumn();
			ImGui::Text("%u", handle.value);

			ImGui::TableNextColumn();
			ImGui::Text("%s", metadata.path.c_str());
		}

		ImGui::EndTable();
	}

	ImGui::EndChild();

	ImGui::Separator();

	if (ImGui::Button("Collect Garbage", ImVec2(-1, 0))) {
		AssetSystem::collect_garbage();
	}
}

std::shared_ptr<Scene> EditorLayer::_get_scene() { return is_running ? runtime_scene : scene; }
