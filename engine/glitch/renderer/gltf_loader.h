/**
 * @file gltf_loader.h
 *
 */

#pragma once

#include "glitch/renderer/scene_graph.h"

namespace tinygltf {
class Node;
class Primitive;
class Model;
class Mesh;
} //namespace tinygltf

class GLTFLoader {
public:
	GLTFLoader();
	~GLTFLoader();

	Ref<SceneNode> load_gltf(const fs::path& p_path);

private:
	void _parse_node(int p_node_idx, const tinygltf::Model* p_model,
			const fs::path& p_base_path, Ref<SceneNode> p_parent_node = nullptr);

	Ref<Mesh> _load_mesh(const tinygltf::Node* p_gltf_node,
			const tinygltf::Model* p_model, const fs::path& p_base_path);

	Ref<MeshPrimitive> _load_primitive(const tinygltf::Primitive* p_primitive,
			const tinygltf::Model* p_model, const tinygltf::Mesh* p_mesh,
			const fs::path& p_base_path);

private:
	Ref<MaterialSystem> material_system;

	Ref<Texture> default_texture;
	Ref<MaterialInstance> default_material;

	std::unordered_map<int, Ref<Texture>> loaded_textures;

	MeshHandle next_handle = 1;
};
