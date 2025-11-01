#pragma once

#include <glitch/core/application.h>
#include <glitch/core/deletion_queue.h>
#include <glitch/renderer/camera.h>
#include <glitch/renderer/render_backend.h>
#include <glitch/scene/entity.h>
#include <glitch/scene/gltf_loader.h>
#include <glitch/scene/scene_renderer.h>

#include "camera_controller.h"
#include "grid_pass.h"

using namespace gl;

class EditorApplication : public Application {
public:
	EditorApplication(const ApplicationCreateInfo& info);
	virtual ~EditorApplication() = default;

protected:
	void _on_start() override;

	void _on_update(float p_dt) override;

	void _on_destroy() override;

private:
	void _render_hierarchy();

	void _render_hierarchy_entry(Entity p_entity);

	void _render_hierarchy_context_menu(const Entity& p_entity);

	void _render_inspector(Entity& p_entity);

	Ref<Scene> _get_scene();

private:
	Ref<SceneRenderer> scene_renderer;

	Ref<Scene> scene;
	Scope<GLTFLoader> gltf_loader;

	CameraController camera_controller;
	UID camera_uid;

	Ref<GridPass> grid_pass;

	Entity selected_entity = INVALID_ENTITY;
	DeletionQueue node_deletion_queue;

	RendererSettings renderer_settings = {};

	// Scripting
	Ref<Scene> runtime_scene;
	bool is_running = false;
};
