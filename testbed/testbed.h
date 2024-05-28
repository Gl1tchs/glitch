#pragma once

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
	Ref<OrthographicCameraNode> camera;
	Ref<GeometryNode> my_node;
	Ref<ComputeEffectNode> effect_node;

	Ref<MetallicRoughnessMaterial> material;

	Ref<Image> color_image;
	Ref<Image> white_image;
};
