/**
 * @file mesh_loader.h
 *
 */

#pragma once

#include "glitch/renderer/mesh.h"

class MeshLoader {
public:
	~MeshLoader();

	MeshHandle load_mesh(const fs::path& p_path);
	Ref<Mesh> get_mesh(MeshHandle p_handle) const;

private:
	std::unordered_map<MeshHandle, Ref<Mesh>> meshes;
	std::unordered_map<int, Ref<Texture>> loaded_textures;

	MeshHandle next_handle = 1;

	Ref<Mesh> load_from_gltf(const fs::path& p_path);
};
