#pragma once

#include <glitch/core/application.h>
#include <glitch/renderer/camera.h>
#include <glitch/renderer/render_backend.h>
#include <glitch/renderer/scene_renderer.h>
#include <glitch/scene/components.h>

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
	Ref<RenderBackend> backend;

	Ref<SceneRenderer> scene_renderer;
	Scene scene;

	CameraController camera_controller;
	CameraComponent* cc;
	Transform* camera_transform;
};
