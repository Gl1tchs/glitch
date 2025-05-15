/**
 * @file mesh_loader.h
 *
 */

#pragma once

#include "glitch/renderer/mesh.h"

class MeshLoader {
public:
	MeshHandle load_mesh(const std::string& path);
	Ref<Mesh> get_mesh(MeshHandle handle) const;

private:
	std::unordered_map<MeshHandle, Ref<Mesh>> meshes;
	MeshHandle next_handle = 1;

	Ref<Mesh> load_from_gltf(const std::string& path);
};
