#pragma once

#include <glitch/core/application.h>
#include <glitch/core/deletion_queue.h>
#include <glitch/renderer/camera.h>
#include <glitch/renderer/render_backend.h>
#include <glitch/scene_graph/gltf_loader.h>
#include <glitch/scene_graph/scene_renderer.h>

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
	void _traverse_render_node_hierarchy(const Ref<SceneNode>& p_node);

	void _render_hierarchy_context_menu(const Ref<SceneNode>& p_node);

	void _render_node_properties(Ref<SceneNode> p_node);

private:
	Ref<SceneRenderer> scene_renderer;

	SceneGraph scene_graph;
	Scope<GLTFLoader> gltf_loader;

	CameraController camera_controller;
	PerspectiveCamera camera;

	Ref<GridPass> grid_pass;

	Ref<SceneNode> selected_node = nullptr;
	DeletionQueue node_deletion_queue;

	RendererSettings renderer_settings = {};
};
