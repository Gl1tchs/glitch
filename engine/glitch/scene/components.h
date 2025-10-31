/**
 * @file components.h
 */

#pragma once

#include "glitch/renderer/camera.h"
#include "glitch/renderer/mesh.h"
#include "glitch/scripting/script_engine.h"

namespace gl {

struct MeshComponent {
	MeshHandle mesh;
	bool visible; // internal scene renderer functionality for frustum culling
};

struct CameraComponent {
	PerspectiveCamera camera;
	bool enabled = true;
};

struct ScriptComponent {
	std::string script_path;
	ScriptRef script;
	bool initialized = false;
};

} //namespace gl