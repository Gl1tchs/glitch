/**
 * @file gltf_loader.h
 *
 */

#pragma once

#include "glitch/renderer/material.h"
#include "glitch/renderer/mesh.h"
#include "glitch/scene/scene.h"

namespace tinygltf {
class Node;
class Primitive;
class Model;
class Mesh;
} //namespace tinygltf

namespace gl {

class GL_API GLTFLoader {
public:
	GLTFLoader();
	~GLTFLoader();

	Result<Entity, std::string> load_gltf(const fs::path& p_path,
			Ref<Scene> p_scene,
			Ref<MaterialInstance> p_overload_material = nullptr);

private:
	void _parse_node(Ref<Scene> p_scene, int p_node_idx,
			const tinygltf::Model* p_model, const size_t p_model_hash,
			const fs::path& p_base_path,
			Ref<MaterialInstance> p_overload_material,
			UID p_parent_id = INVALID_UID);

	Ref<Mesh> _load_mesh(const tinygltf::Node* p_gltf_node,
			const tinygltf::Model* p_model, const size_t p_model_hash,
			const fs::path& p_base_path,
			Ref<MaterialInstance> p_overload_material);

	Ref<MeshPrimitive> _load_primitive(const tinygltf::Primitive* p_primitive,
			const tinygltf::Model* p_model, const size_t p_model_hash,
			const tinygltf::Mesh* p_mesh, const fs::path& p_base_path,
			Ref<MaterialInstance> p_overload_material);

	Ref<Texture> _load_texture(int texture_index,
			const tinygltf::Model* p_model, const size_t p_model_hash,
			const fs::path& p_base_path);

private:
	Ref<Texture> default_texture;
	Ref<MaterialInstance> default_material;

	// model + texture_index hash = texture
	std::unordered_map<size_t, Ref<Texture>> loaded_textures;
};

} //namespace gl