#pragma once

#include <glitch/core/deletion_queue.h>
#include <glitch/core/layer.h>
#include <glitch/renderer/camera.h>
#include <glitch/renderer/render_backend.h>
#include <glitch/scene/entity.h>
#include <glitch/scene/scene_renderer.h>
#include <functional>

#include "camera_controller.h"
#include "grid_pass.h"

using namespace gl;

class EditorLayer : public Layer {
public:
	virtual ~EditorLayer() = default;

	void start() override;
	void update(float p_dt) override;
	void destroy() override;

private:
	// --- UI Panels ---
	void _begin_dockspace();
	void _end_dockspace();
	void _render_menubar();
	void _render_viewport(float p_dt);
	void _render_stats();
	void _render_settings();
	void _render_hierarchy();
	void _render_inspector(Entity& p_entity);
	void _render_asset_registry();
	void _render_script_panel();

	// --- Helpers ---
	void _render_hierarchy_entry(Entity p_entity);
	void _render_hierarchy_context_menu(Entity p_entity);

	// Helper to reduce boilerplate in inspector
	template <typename T>
	void _draw_component(
			const std::string& name, Entity& entity, std::function<void(T&)> ui_function);

	std::shared_ptr<Scene> _get_scene();

private:
	std::shared_ptr<SceneRenderer> scene_renderer;

	std::shared_ptr<Scene> scene;
	std::optional<fs::path> scene_path = std::nullopt;

	CameraController camera_controller;
	std::shared_ptr<GridPass> grid_pass;

	Entity selected_entity = INVALID_ENTITY;
	DeletionQueue frame_deletion_queue;

	RendererSettings renderer_settings = {};

	// Scripting
	std::shared_ptr<Scene> runtime_scene;
	bool is_running = false;

	// UI State
	bool show_grid = false;

	// Stores descriptors so we don't re-upload every frame.
	UID selected_id = 0; // so that we can free thumbnails on change
	std::unordered_map<uint64_t, void*> thumb_texture_descriptors;
};