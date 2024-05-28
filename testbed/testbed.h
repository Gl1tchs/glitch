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
	OrthographicCamera camera;

	Ref<Mesh> mesh;
	Ref<MetallicRoughnessMaterial> material;

	Ref<MaterialInstance> material_instance;
	Ref<MaterialInstance> material_instance2;

	Ref<Image> color_image;
	Ref<Image> color_image2;
	Ref<Image> white_image;

	Ref<ComputeEffect> effect;
};
