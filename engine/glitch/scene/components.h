/**
 * @file components.h
 */

#pragma once

#include "glitch/asset/asset_system.h"
#include "glitch/renderer/camera.h"

namespace gl {

struct MeshComponent {
	AssetHandle mesh;
	bool visible; // internal scene renderer functionality for frustum culling
};

struct CameraComponent {
	PerspectiveCamera camera;
	bool enabled = true;
};

GL_DEFINE_SERIALIZABLE(CameraComponent, camera, enabled);

} //namespace gl
