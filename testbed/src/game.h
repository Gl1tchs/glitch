#pragma once

#include <glitch/core/application.h>
#include <glitch/renderer/camera.h>
#include <glitch/renderer/render_backend.h>
#include <glitch/renderer/scene_renderer.h>

#include "camera_controller.h"
#include "example.h"

class Game : public Application {
public:
	Game(const ApplicationCreateInfo& info);
	virtual ~Game() = default;

protected:
	void _on_start() override;

	void _on_update(float p_dt) override;

	void _on_destroy() override;

private:
	void _render_examples(float p_dt);

private:
	Ref<RenderBackend> backend;

	Ref<SceneRenderer> scene_renderer;
	Scene scene;

	CameraController camera_controller;
	PerspectiveCamera camera;
	Transform camera_transform;

	bool draw_examples = false;
	std::vector<Ref<Example>> examples;
	Ref<Example> active_example;
};
