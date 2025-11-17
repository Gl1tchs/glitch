/**
 * @file components.h
 */

#pragma once

#include "glitch/asset/asset_system.h"
#include "glitch/renderer/camera.h"
#include "glitch/scripting/script_engine.h"

namespace gl {

struct MeshComponent {
	AssetHandle mesh;
	bool visible; // internal scene renderer functionality for frustum culling
};

struct CameraComponent {
	PerspectiveCamera camera;
	bool enabled = true;
};

struct ScriptComponent {
	std::string script_path;

	ScriptRef script = 0;
	bool is_loaded = false;

	// metadata cache for reloading
	std::optional<ScriptMetadata> metadata = std::nullopt;

	ScriptResult load();

	void unload();

	void reset();
};

} //namespace gl