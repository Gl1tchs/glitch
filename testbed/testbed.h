#pragma once

#include <core/application.h>
#include <renderer/camera.h>
#include <renderer/model.h>
#include <scene/scene.h>

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

	void _draw_entity(const Entity p_entity);

	void _draw_inspector();

	void _draw_stats(float dt);

	void _draw_settings();

private:
	Scene scene;

	Entity camera_entity;

	Ref<Model> plane;
	Ref<Model> gentelman;
	Ref<Model> floor;

	Ref<Material> material;
	Ref<Grid> grid;

	CameraController camera_controller;
	PerspectiveCamera camera;

	Entity selected_entity = INVALID_ENTITY;

	bool draw_grid = false;
};
