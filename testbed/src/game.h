#pragma once

#include <glitch/core/application.h>
#include <glitch/renderer/camera.h>
#include <glitch/renderer/render_backend.h>
#include <glitch/scene_graph/gltf_loader.h>
#include <glitch/scene_graph/scene_renderer.h>

#include "camera_controller.h"

struct GridPushConstants {
	glm::mat4 view_proj;
	glm::vec3 camera_pos;
	float grid_size;
};

class Game : public Application {
public:
	Game(const ApplicationCreateInfo& info);
	virtual ~Game() = default;

protected:
	void _on_start() override;

	void _on_update(float p_dt) override;

	void _on_destroy() override;

private:
	Ref<SceneRenderer> renderer;

	SceneGraph scene_graph;

	GLTFLoader gltf_loader;
	std::string model_path = "";

	CameraController camera_controller;
	PerspectiveCamera camera;

	Shader grid_shader;
	Pipeline grid_pipeline;
};
