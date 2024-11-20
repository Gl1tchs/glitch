#pragma once

#include "renderer/camera.h"

struct TagComponent {
	std::string name;
};

struct CameraComponent {
	PerspectiveCamera camera;
	bool is_active = true;
};
