#pragma once

#include "camera_controller.h"

#include <gl/core/application.h>
#include <gl/core/timer.h>
#include <gl/renderer/camera.h>
#include <gl/renderer/compute.h>
#include <gl/renderer/material.h>
#include <gl/renderer/mesh.h>

class TestBedApplication : public Application {
public:
	TestBedApplication(const ApplicationCreateInfo& info);
	virtual ~TestBedApplication();

protected:
	void _on_start() override;

	void _on_update(float dt) override;

	void _on_destroy() override;

private:
	Ref<MetallicRoughnessMaterial> material;
	Ref<Node> scene;

	CameraController camera_controller;
	Ref<PerspectiveCameraNode> camera;
};
