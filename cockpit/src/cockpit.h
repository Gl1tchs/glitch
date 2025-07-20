#pragma once

#include <glitch/core/application.h>
#include <glitch/renderer/camera.h>
#include <glitch/renderer/render_backend.h>
#include <glitch/scene_graph/gltf_loader.h>
#include <glitch/scene_graph/scene_renderer.h>

#include "camera_controller.h"
#include "grid_pass.h"

using namespace gl;

class CockpitApplication : public Application {
public:
	CockpitApplication(const ApplicationCreateInfo& info);
	virtual ~CockpitApplication() = default;

protected:
	void _on_start() override;

	void _on_update(float p_dt) override;

	void _on_destroy() override;

private:
	Ref<SceneRenderer> scene_renderer;

	SceneGraph scene_graph;

	Ref<GridPass> grid_pass;

	Scope<GLTFLoader> gltf_loader;
	std::string model_path = "";

	CameraController camera_controller;
	PerspectiveCamera camera;
};
