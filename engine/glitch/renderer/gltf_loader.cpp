#include "glitch/renderer/gltf_loader.h"

#include "glitch/renderer/materials/material_unlit.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>

enum GLTFParsingFlags : uint16_t {
	GLTF_PARSING_FLAG_NO_UV = 0x1,
	GLTF_PARSING_FLAG_NO_NORMALS = 0x2,
};

GLTFLoader::GLTFLoader() {
	default_texture = Texture::create(COLOR_WHITE, { 1, 1 });

	material_system = create_ref<MaterialSystem>();
	material_system->register_definition(
			"mat_unlit", get_unlit_material_definition());

	default_material = material_system->create_instance("mat_unlit");
	default_material->set_param("base_color", COLOR_WHITE);
	default_material->set_param("metallic", 0.5f);
	default_material->set_param("roughness", 0.5f);
	default_material->set_param("u_albedo_texture", default_texture);
	default_material->upload();
}

GLTFLoader::~GLTFLoader() { loaded_textures.clear(); }

Ref<SceneNode> GLTFLoader::load_gltf(const fs::path& p_path) {
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
		ret = loader.LoadASCIIFromFile(&model, &err, &warn, p_path.string());
	}

	GL_ASSERT(ret, err.c_str());

	Ref<SceneNode> base_node = create_ref<SceneNode>();
	const auto base_path = p_path.parent_path();

	const tinygltf::Scene& scene = model.scenes[model.defaultScene];
	for (int node_index : scene.nodes) {
		_parse_node(node_index, &model, base_path, base_node);
	}

	return base_node;
}

void GLTFLoader::_parse_node(int p_node_idx, const tinygltf::Model* p_model,
		const fs::path& p_base_path, Ref<SceneNode> p_parent_node) {
	Ref<SceneNode> node = create_ref<SceneNode>();
	if (p_parent_node) {
		p_parent_node->add_child(node);
	}

	// Create and load mesh primitive if exists
	const tinygltf::Node& gltf_node = p_model->nodes[p_node_idx];
	if (gltf_node.mesh >= 0) {
		node->mesh = _load_mesh(&gltf_node, p_model, p_base_path);
	}

	for (int child_node_idx : gltf_node.children) {
		_parse_node(child_node_idx, p_model, p_base_path, node);
	}
}

Ref<Mesh> GLTFLoader::_load_mesh(const tinygltf::Node* p_gltf_node,
		const tinygltf::Model* p_model, const fs::path& p_base_path) {
	Ref<Mesh> mesh = create_ref<Mesh>();

	const tinygltf::Mesh& gltf_mesh = p_model->meshes[p_gltf_node->mesh];
	for (const auto& primitive : gltf_mesh.primitives) {
		mesh->primitives.push_back(
				_load_primitive(&primitive, p_model, &gltf_mesh, p_base_path));
	}

	return mesh;
}

Ref<MeshPrimitive> GLTFLoader::_load_primitive(
		const tinygltf::Primitive* p_primitive, const tinygltf::Model* p_model,
		const tinygltf::Mesh* p_mesh, const fs::path& p_base_path) {
	uint16_t parsing_flags = 0;

	const auto& pos_accessor =
			p_model->accessors[p_primitive->attributes.at("POSITION")];
	const auto& pos_view = p_model->bufferViews[pos_accessor.bufferView];
	const auto& pos_buffer = p_model->buffers[pos_view.buffer];

	const auto& index_accessor = p_model->accessors[p_primitive->indices];
	const auto& index_view = p_model->bufferViews[index_accessor.bufferView];
	const auto& index_buffer = p_model->buffers[index_view.buffer];

	// UV and Normal values could be non existing so we need to check that first

	uint32_t uv_accessor_offset;
	uint32_t uv_view_offset;
	const tinygltf::Buffer* uv_buffer = nullptr;
	if (p_primitive->attributes.find("TEXCOORD_0") !=
			p_primitive->attributes.end()) {
		const auto& uv_accessor =
				p_model->accessors[p_primitive->attributes.at("TEXCOORD_0")];
		const auto& uv_view = p_model->bufferViews[uv_accessor.bufferView];

		uv_accessor_offset = uv_accessor.byteOffset;
		uv_view_offset = uv_view.byteOffset;
		uv_buffer = &p_model->buffers[uv_view.buffer];
	} else {
		parsing_flags |= GLTF_PARSING_FLAG_NO_UV;
	}

	uint32_t normal_accessor_offset;
	uint32_t normal_view_offset;
	const tinygltf::Buffer* normal_buffer = nullptr;
	if (p_primitive->attributes.find("NORMAL") !=
			p_primitive->attributes.end()) {
		const auto& normal_accessor =
				p_model->accessors[p_primitive->attributes.at("NORMAL")];
		const auto& normal_view =
				p_model->bufferViews[normal_accessor.bufferView];

		normal_accessor_offset = normal_accessor.byteOffset;
		normal_view_offset = normal_view.byteOffset;
		normal_buffer = &p_model->buffers[normal_view.buffer];
	} else {
		parsing_flags |= GLTF_PARSING_FLAG_NO_NORMALS;
	}

	// Vertices
	const size_t vertex_count = pos_accessor.count;
	std::vector<MeshVertex> prim_vertices(vertex_count);

	constexpr float DEFAULT_NORMAL_DATA[3] = { 0, 0, 0 };
	constexpr float DEFAULT_UV_DATA[3] = { 0, 0 };

	for (size_t i = 0; i < vertex_count; ++i) {
		const float* pos = reinterpret_cast<const float*>(
				&pos_buffer.data[pos_view.byteOffset + pos_accessor.byteOffset +
						i * 12]);

		const float* uv = (parsing_flags & GLTF_PARSING_FLAG_NO_UV)
				? DEFAULT_UV_DATA
				: reinterpret_cast<const float*>(
						  &uv_buffer->data[uv_view_offset + uv_accessor_offset +
								  i * 8]);

		const float* normal = (parsing_flags & GLTF_PARSING_FLAG_NO_NORMALS)
				? DEFAULT_NORMAL_DATA
				: reinterpret_cast<const float*>(
						  &normal_buffer->data[normal_view_offset +
								  normal_accessor_offset + i * 12]);

		prim_vertices[i] = {
			glm::vec3(pos[0], pos[1], pos[2]),
			uv[0],
			glm::vec3(normal[0], normal[1], normal[2]),
			uv[1],
		};
	}

	// Indices
	const size_t index_count = index_accessor.count;
	std::vector<uint32_t> prim_indices(index_count);

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
	Ref<MaterialInstance> material = default_material;

	// Create material instance
	if (p_primitive->material >= 0 &&
			p_primitive->material < p_model->materials.size()) {
		const tinygltf::Material& gltf_material =
				p_model->materials[p_primitive->material];
		const auto& base_color =
				gltf_material.pbrMetallicRoughness.baseColorFactor;

		material = material_system->create_instance("mat_unlit");
		material->set_param("base_color",
				Color(base_color[0], base_color[1], base_color[2],
						base_color[3]));
		material->set_param("metallic",
				static_cast<float>(
						gltf_material.pbrMetallicRoughness.metallicFactor));
		material->set_param("roughness",
				static_cast<float>(
						gltf_material.pbrMetallicRoughness.roughnessFactor));

		// Load textures
		Ref<Texture> texture = default_texture;

		const int texture_index =
				gltf_material.pbrMetallicRoughness.baseColorTexture.index;
		if (texture_index > 0) {
			if (loaded_textures.find(texture_index) != loaded_textures.end()) {
				// Texture already loaded

				texture = loaded_textures.at(texture_index);
			} else {
				// Texture not loaded

				const tinygltf::Texture& gltf_texture =
						p_model->textures[texture_index];
				const tinygltf::Image& gltf_image =
						p_model->images[gltf_texture.source];

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

					texture = Texture::create(format,
							glm::uvec2(gltf_image.width, gltf_image.height),
							gltf_image.image.data());
				} else {
					// External file — use loader
					texture = Texture::load_from_path(
							p_base_path / gltf_image.uri);
				}

				loaded_textures[texture_index] = texture;
			}
		}

		material->set_param("u_albedo_texture", texture);
		material->upload();
	}

	Ref<MeshPrimitive> prim =
			MeshPrimitive::create(prim_vertices, prim_indices);
	prim->material = material;

	return prim;
}
