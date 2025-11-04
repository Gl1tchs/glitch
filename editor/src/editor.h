#pragma once

#include <glitch/core/deletion_queue.h>
#include <glitch/core/layer.h>
#include <glitch/renderer/camera.h>
#include <glitch/renderer/render_backend.h>
#include <glitch/scene/entity.h>
#include <glitch/scene/gltf_loader.h>
#include <glitch/scene/scene_renderer.h>

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
	void _render_hierarchy();

	void _render_hierarchy_entry(Entity p_entity);

	void _render_hierarchy_context_menu(Entity p_entity);

	void _render_inspector(Entity& p_entity);

	std::shared_ptr<Scene> _get_scene();

private:
	std::shared_ptr<SceneRenderer> scene_renderer;

	std::shared_ptr<Scene> scene;
	std::unique_ptr<GLTFLoader> gltf_loader;

	CameraController camera_controller;
	UID camera_uid;

	std::shared_ptr<GridPass> grid_pass;

	Entity selected_entity = INVALID_ENTITY;
	DeletionQueue node_deletion_queue;

	RendererSettings renderer_settings = {};

	// Scripting
	std::shared_ptr<Scene> runtime_scene;
	bool is_running = false;
};
