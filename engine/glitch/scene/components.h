/**
 * @file components.h
 */

#pragma once

#include "glitch/asset/asset_system.h"
#include "glitch/renderer/camera.h"
#include "glitch/renderer/material.h"

namespace gl {

struct MeshComponent {
	AssetHandle mesh;
	bool visible; // internal scene renderer functionality for frustum culling
};

struct MaterialComponent {
	AssetHandle handle;
	std::string definition_path;

	std::unordered_map<std::string, ShaderUniformVariable> uniforms;
};

struct CameraComponent {
	PerspectiveCamera camera;
	bool enabled = true;
};

GL_DEFINE_SERIALIZABLE(CameraComponent, camera, enabled);

} //namespace gl
