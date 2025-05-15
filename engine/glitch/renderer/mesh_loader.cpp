#include "glitch/renderer/mesh_loader.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>

MeshHandle MeshLoader::load_mesh(const std::string& path) {
	Ref<Mesh> mesh = load_from_gltf(path);
	if (!mesh) {
		return MeshHandle();
	}

	MeshHandle handle = static_cast<MeshHandle>(next_handle++);
	meshes[handle] = mesh;
	return handle;
}

Ref<Mesh> MeshLoader::get_mesh(MeshHandle handle) const {
	auto it = meshes.find(handle);
	if (it == meshes.end())
		return nullptr;
	return it->second;
}

Ref<Mesh> MeshLoader::load_from_gltf(const std::string& path) {
	tinygltf::Model model;
	tinygltf::TinyGLTF loader;
	std::string err, warn;

	bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, path);
	GL_ASSERT(ret, err.c_str());

	Ref<Mesh> mesh = create_ref<Mesh>();
	mesh->name = model.scenes[model.defaultScene].name;

	const tinygltf::Scene& scene = model.scenes[model.defaultScene];
	for (int node_index : scene.nodes) {
		const tinygltf::Node& node = model.nodes[node_index];
		if (node.mesh < 0) {
			continue;
		}

		std::vector<MeshVertex> prim_vertices;
		std::vector<uint32_t> prim_indices;

		const tinygltf::Mesh& gltf_mesh = model.meshes[node.mesh];
		for (const auto& primitive : gltf_mesh.primitives) {
			prim_vertices.clear();
			prim_indices.clear();

			const auto& pos_accessor =
					model.accessors[primitive.attributes.at("POSITION")];
			const auto& normal_accessor =
					model.accessors[primitive.attributes.at("NORMAL")];
			const auto& uv_accessor =
					model.accessors[primitive.attributes.at("TEXCOORD_0")];
			const auto& index_accessor = model.accessors[primitive.indices];

			const auto& pos_view = model.bufferViews[pos_accessor.bufferView];
			const auto& normal_view =
					model.bufferViews[normal_accessor.bufferView];
			const auto& uv_view = model.bufferViews[uv_accessor.bufferView];
			const auto& index_view =
					model.bufferViews[index_accessor.bufferView];

			const auto& pos_buffer = model.buffers[pos_view.buffer];
			const auto& normal_buffer = model.buffers[normal_view.buffer];
			const auto& uv_buffer = model.buffers[uv_view.buffer];
			const auto& index_buffer = model.buffers[index_view.buffer];

			size_t vertex_count = pos_accessor.count;
			prim_vertices.resize(vertex_count);

			for (size_t i = 0; i < vertex_count; ++i) {
				const float* pos = reinterpret_cast<const float*>(
						&pos_buffer.data[pos_view.byteOffset +
								pos_accessor.byteOffset + i * 12]);
				const float* normal = reinterpret_cast<const float*>(
						&normal_buffer.data[normal_view.byteOffset +
								normal_accessor.byteOffset + i * 12]);
				const float* uv = reinterpret_cast<const float*>(
						&uv_buffer.data[uv_view.byteOffset +
								uv_accessor.byteOffset + i * 8]);

				prim_vertices[i] = { glm::vec3(pos[0], pos[1], pos[2]), uv[0],
					glm::vec3(normal[0], normal[1], normal[2]), uv[1] };
			}

			size_t index_count = index_accessor.count;
			prim_indices.resize(index_count);
			if (index_accessor.componentType ==
					TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
				const uint16_t* indices = reinterpret_cast<const uint16_t*>(
						&index_buffer.data[index_view.byteOffset +
								index_accessor.byteOffset]);
				for (size_t i = 0; i < index_count; ++i) {
					prim_indices[i] = static_cast<uint32_t>(indices[i]);
				}
			} else if (index_accessor.componentType ==
					TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
				const uint32_t* indices = reinterpret_cast<const uint32_t*>(
						&index_buffer.data[index_view.byteOffset +
								index_accessor.byteOffset]);
				for (size_t i = 0; i < index_count; ++i) {
					prim_indices[i] = indices[i];
				}
			} else {
				GL_ASSERT(false, "Unsupported index type");
			}

			Ref<MeshPrimitive> prim =
					MeshPrimitive::create(prim_vertices, prim_indices);
			prim->name = gltf_mesh.name;

			mesh->primitives.push_back(prim);
		}
	}

	return mesh;
}