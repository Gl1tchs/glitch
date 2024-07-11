#pragma once

#include "renderer/camera.h"
#include "renderer/model.h"

struct TagComponent {
	std::string name;
};

struct CameraComponent {
	PerspectiveCamera camera;
	bool is_primary = true;
};

struct MeshRendererComponent {
	Ref<Model> model;
};
