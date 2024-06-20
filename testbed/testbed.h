#pragma once

#include <core/application.h>
#include <renderer/camera.h>
#include <renderer/scene_graph.h>

#include "camera_controller.h"
#include "grid.h"

class TestBedApplication : public Application {
public:
	TestBedApplication(const ApplicationCreateInfo& info);
	virtual ~TestBedApplication() = default;

protected:
	void _on_start() override;

	void _on_update(float p_dt) override;

	void _on_destroy() override;

private:
	void _imgui_render(float p_dt);

	void _draw_hierarchy();

	void _draw_node(const Ref<Node> p_node);

	void _draw_inspector();

	void _draw_stats(float dt);

	void _draw_settings();

private:
	SceneGraph scene_graph;

	Ref<Material> material;
	Ref<Grid> grid;

	CameraController camera_controller;
	Ref<PerspectiveCameraNode> camera;

	Ref<Node> selected_node = nullptr;

	bool draw_grid = true;
};
