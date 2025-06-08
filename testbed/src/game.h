#pragma once

#include <glitch/core/application.h>
#include <glitch/renderer/camera.h>
#include <glitch/renderer/render_backend.h>
#include <glitch/renderer/renderer.h>

#include "camera_controller.h"

class Game : public Application {
public:
	Game(const ApplicationCreateInfo& info);
	virtual ~Game() = default;

protected:
	void _on_start() override;

	void _on_update(float p_dt) override;

	void _on_destroy() override;

private:
	Ref<Renderer> renderer;

	SceneGraph scene_graph;

	GLTFLoader gltf_loader;
	std::string model_path = "";

	CameraController camera_controller;
	PerspectiveCamera camera;
	Transform camera_transform;
};
