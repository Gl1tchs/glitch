#pragma once

#include <glitch/core/application.h>
#include <glitch/renderer/camera.h>
#include <glitch/renderer/render_backend.h>
#include <glitch/renderer/renderer.h>
#include <glitch/scene_graph/gltf_loader.h>

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
	Ref<Renderer> renderer;

	SceneGraph scene_graph;

	Future<Ref<SceneNode>> scene_fut;

	GLTFLoader gltf_loader;
	std::string model_path = "";

	CameraController camera_controller;
	PerspectiveCamera camera;
	Transform camera_transform;

	RenderPass grid_pass;
	std::vector<FrameBuffer> grid_fbs;
	Shader grid_shader;
	Pipeline grid_pipeline;
};
