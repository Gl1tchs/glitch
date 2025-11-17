/**
 * @file gltf_loader.h
 *
 */

#pragma once

#include "glitch/scene/scene.h"

namespace gl {

enum class GLTFLoadError {
	NONE,
	INVALID_EXTENSION,
	PARSING_ERROR,
};

/**
 * GLTF loader, loads and registers GLTF models to the given scene from path.
 *
 */
struct GLTFLoader {
	static GLTFLoadError load(std::shared_ptr<Scene> p_scene, const fs::path& p_path);
};

} //namespace gl