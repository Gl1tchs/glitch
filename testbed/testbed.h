#pragma once

#include "camera_controller.h"

#include <gl/core/application.h>
#include <gl/core/timer.h>
#include <gl/renderer/camera.h>
#include <gl/renderer/compute.h>
#include <gl/renderer/material.h>
#include <gl/renderer/model.h>

class TestBedApplication : public Application {
public:
	TestBedApplication(const ApplicationCreateInfo& info);
	virtual ~TestBedApplication();

protected:
	void _on_start() override;

	void _on_update(float dt) override;

	void _on_destroy() override;

private:
	std::vector<Ref<Model>> models;

	CameraController camera_controller;
	Ref<PerspectiveCameraNode> camera;

	Ref<ComputeEffectNode> effect_node;

	Ref<MetallicRoughnessMaterial> material;

	Ref<Image> color_image;
	Ref<Image> white_image;
};
