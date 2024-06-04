#pragma once

#include "camera_controller.h"

#include <core/application.h>
#include <core/timer.h>
#include <renderer/camera.h>

class TestBedApplication : public Application {
public:
	TestBedApplication(const ApplicationCreateInfo& info);
	virtual ~TestBedApplication() = default;

protected:
	void _on_start() override;

	void _on_update(float dt) override;

	void _on_destroy() override;

private:
	Ref<Node> scene;

	CameraController camera_controller;
	Ref<PerspectiveCameraNode> camera;
};
