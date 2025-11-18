/**
 * @file gltf_loader.h
 *
 */

#pragma once

#include "glitch/scene/scene.h"

namespace gl {

struct GLTFSourceComponent {
	UID model_id;
	std::string asset_path;
	// TODO: load options
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(GLTFSourceComponent, model_id, asset_path);

/**
 * Component representing an entity, loaded from a
 * GLTF scene.
 *
 */
struct GLTFInstanceComponent {
	UID source_model_id;
	int gltf_node_id;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(GLTFInstanceComponent, source_model_id, gltf_node_id);

enum class GLTFLoadError {
	NONE,
	INVALID_EXTENSION,
	PARSING_ERROR,
	PATH_ERROR,
};

/**
 * GLTF loader, loads and registers GLTF models to the given scene from path.
 *
 */
struct GL_API GLTFLoader {
	static GLTFLoadError load(std::shared_ptr<Scene> p_scene, const std::string& p_path);
};

} //namespace gl