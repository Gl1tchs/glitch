#include "glitch/renderer/mesh_loader.h"

#include "glitch/renderer/render_backend.h"
#include "glitch/renderer/renderer.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>

MeshLoader::~MeshLoader() {
	Ref<RenderBackend> backend = Renderer::get_backend();

	loaded_textures.clear();
	meshes.clear();
}

MeshHandle MeshLoader::load_mesh(const fs::path& p_path) {
	Ref<Mesh> mesh = load_from_gltf(p_path);
	if (!mesh) {
		return MeshHandle();
	}

	MeshHandle handle = static_cast<MeshHandle>(next_handle++);
	meshes[handle] = mesh;
	return handle;
}

Ref<Mesh> MeshLoader::get_mesh(MeshHandle p_handle) const {
	auto it = meshes.find(p_handle);
	if (it == meshes.end()) {
		return nullptr;
	}
	return it->second;
}

Ref<Mesh> MeshLoader::load_from_gltf(const fs::path& p_path) {
	// TODO: better validation
	GL_ASSERT(p_path.has_extension() && p_path.extension() == ".glb" ||
			p_path.extension() == ".gltf")

	tinygltf::Model model;
	tinygltf::TinyGLTF loader;
	std::string err, warn;

	bool ret;
	if (p_path.extension() == ".glb") {
		ret = loader.LoadBinaryFromFile(&model, &err, &warn, p_path.string());
	} else {
		ret = loader.LoadBinaryFromFile(&model, &err, &warn, p_path.string());
	}

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

			// Vertices
			const size_t vertex_count = pos_accessor.count;
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

				prim_vertices[i] = {
					glm::vec3(pos[0], pos[1], pos[2]),
					uv[0],
					glm::vec3(normal[0], normal[1], normal[2]),
					uv[1],
				};
			}

			// Indices
			const size_t index_count = index_accessor.count;
			prim_indices.resize(index_count);

			switch (index_accessor.componentType) {
				case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
					const uint16_t* indices = reinterpret_cast<const uint16_t*>(
							&index_buffer.data[index_view.byteOffset +
									index_accessor.byteOffset]);
					for (size_t i = 0; i < index_count; ++i) {
						prim_indices[i] = static_cast<uint32_t>(indices[i]);
					}

					break;
				}
				case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
					const uint32_t* indices = reinterpret_cast<const uint32_t*>(
							&index_buffer.data[index_view.byteOffset +
									index_accessor.byteOffset]);
					for (size_t i = 0; i < index_count; ++i) {
						prim_indices[i] = indices[i];
					}

					break;
				}
				default:
					GL_ASSERT(false, "Unsupported index type");
			}

			// Material
			MeshMaterialParameters material_params = {};

			if (primitive.material >= 0 &&
					primitive.material < model.materials.size()) {
				const tinygltf::Material& gltf_material =
						model.materials[primitive.material];

				const auto& base_color =
						gltf_material.pbrMetallicRoughness.baseColorFactor;

				material_params.base_color = Color(base_color[0], base_color[1],
						base_color[2], base_color[3]);
				material_params.metallic =
						gltf_material.pbrMetallicRoughness.metallicFactor;
				material_params.roughness =
						gltf_material.pbrMetallicRoughness.roughnessFactor;

				// Load textures
				const int texture_index = gltf_material.pbrMetallicRoughness
												  .baseColorTexture.index;
				if (texture_index > 0) {
					if (loaded_textures.find(texture_index) !=
							loaded_textures.end()) {
						// Texture already loaded

						material_params.albedo_texture =
								loaded_textures.at(texture_index);
					} else {
						// Texture not loaded

						const tinygltf::Texture& gltf_texture =
								model.textures[texture_index];
						const tinygltf::Image& gltf_image =
								model.images[gltf_texture.source];

						if (gltf_image.uri.empty()) {
							// Embedded — create directly from buffer

							DataFormat format;
							switch (gltf_image.component) {
								case 1:
									format = DATA_FORMAT_R8_UNORM;
									break;
								case 2:
									format = DATA_FORMAT_R8G8_UNORM;
									break;
								case 3:
									format = DATA_FORMAT_R8G8B8_UNORM;
									break;
								case 4:
									format = DATA_FORMAT_R8G8B8A8_UNORM;
									break;
								default:
									GL_ASSERT(false,
											"Unsupported image component "
											"count");
							}

							material_params.albedo_texture =
									Texture::create(format,
											glm::uvec2(gltf_image.width,
													gltf_image.height),
											gltf_image.image.data());
						} else {
							const fs::path gltf_dir = p_path.parent_path();

							// External file — use loader
							material_params.albedo_texture =
									Texture::load_from_path(
											gltf_dir / gltf_image.uri);
						}

						loaded_textures[texture_index] =
								material_params.albedo_texture;
					}
				}
			}

			Ref<MeshPrimitive> prim =
					MeshPrimitive::create(prim_vertices, prim_indices);
			prim->name = gltf_mesh.name;
			prim->material = material_params;

			mesh->primitives.push_back(prim);
		}
	}

	return mesh;
}