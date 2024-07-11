#include "renderer/model.h"

#include "renderer/material.h"
#include "renderer/render_backend.h"
#include "renderer/renderer.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>

template <typename T> struct GLTFAccessor {
	const tinygltf::Accessor* accessor;
	const T* data;

	inline const T* get(const int idx) {
		return reinterpret_cast<const T*>(data + (idx * accessor->type));
	}
};

template <typename T>
static const T* _get_gltf_accessor_data(
		const tinygltf::Model& model, const tinygltf::Accessor* accessor) {
	const tinygltf::BufferView& view = model.bufferViews[accessor->bufferView];
	const tinygltf::Buffer& buffer = model.buffers[view.buffer];

	const T* data = reinterpret_cast<const T*>(
			&buffer.data[accessor->byteOffset + view.byteOffset]);
	return data;
}

template <typename T>
static Optional<GLTFAccessor<T>> _get_gltf_accessor(
		const tinygltf::Model& model, const tinygltf::Primitive& primitive,
		const std::string& name) {
	const bool accessor_exists =
			primitive.attributes.find(name) != primitive.attributes.end();

	if (!accessor_exists) {
		return {};
	}

	const auto& gltf_accessor = model.accessors[primitive.attributes.at(name)];
	const auto* gltf_data =
			_get_gltf_accessor_data<float>(model, &gltf_accessor);

	GLTFAccessor accessor = {
		.accessor = &gltf_accessor,
		.data = gltf_data,
	};

	return accessor;
}

struct LoadedImage {
	Image image;
	uint32_t usage_count;
};

// hash, image
static std::unordered_map<size_t, LoadedImage> s_loaded_images;

static size_t _calculate_material_hash(int p_mesh_index, int p_texture_index) {
	return static_cast<size_t>(p_mesh_index) ^
			(static_cast<size_t>(p_texture_index) << 1);
}

static Image _load_material_image(const tinygltf::Model& p_model,
		int p_model_index, int p_texture_index) {
	if (p_texture_index < 0) {
		return nullptr;
	}

	const size_t hash =
			_calculate_material_hash(p_model_index, p_texture_index);

	const auto it = s_loaded_images.find(hash);
	if (it != s_loaded_images.end()) {
		LoadedImage& loaded_image = it->second;
		loaded_image.usage_count++;

		return loaded_image.image;
	}

	const auto& gltf_texture = p_model.textures[p_texture_index];
	const auto& gltf_image = p_model.images[gltf_texture.source];

	const Vec2u size = {
		static_cast<uint32_t>(gltf_image.height),
		static_cast<uint32_t>(gltf_image.width),
	};

	Ref<RenderBackend> backend = Renderer::get_backend();

	Image image = backend->image_create(
			DATA_FORMAT_R8G8B8A8_UNORM, size, (void*)gltf_image.image.data());

	s_loaded_images[hash] = LoadedImage{
		.image = image,
		.usage_count = 1,
	};

	return image;
}

static Ref<Mesh> _process_mesh(const tinygltf::Model& p_model,
		const tinygltf::Primitive& p_primitive, const fs::path& p_directory,
		Ref<Material> p_material, const std::string& p_name,
		int p_model_index) {
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	// position accessor is guaranteed to exists but the others are not
	GLTFAccessor position_accessor =
			_get_gltf_accessor<float>(p_model, p_primitive, "POSITION").value();

	Optional<GLTFAccessor<float>> tex_coord_accessor =
			_get_gltf_accessor<float>(p_model, p_primitive, "TEXCOORD_0");
	Optional<GLTFAccessor<float>> normal_accessor =
			_get_gltf_accessor<float>(p_model, p_primitive, "NORMAL");

	static constexpr glm::vec2 DEFAULT_TEX_COORD = { 0.0f, 0.0f };
	static constexpr glm::vec3 DEFAULT_NORMAL = { 0.0f, 0.0f, 0.0f };

	for (size_t i = 0; i < position_accessor.accessor->count; ++i) {
		Vertex v;
		{
			memcpy(&v.position, position_accessor.get(i), sizeof(glm::vec3));

			if (tex_coord_accessor.has_value()) {
				const float* tex_coord = tex_coord_accessor->get(i);
				v.uv_x = tex_coord[0];
				v.uv_y = tex_coord[1];
			} else {
				v.uv_x = DEFAULT_TEX_COORD.x;
				v.uv_y = DEFAULT_TEX_COORD.y;
			}

			if (normal_accessor.has_value()) {
				memcpy(&v.normal, normal_accessor->get(i), sizeof(glm::vec3));
			} else {
				v.normal = DEFAULT_NORMAL;
			}
		}

		vertices.push_back(v);
	}

	// Accessing indices
	const tinygltf::Accessor& indices_accessor =
			p_model.accessors[p_primitive.indices];
	const tinygltf::BufferView& indices_view =
			p_model.bufferViews[indices_accessor.bufferView];

	const tinygltf::Buffer& indices_buffer =
			p_model.buffers[indices_view.buffer];

	const size_t indices_offset =
			indices_accessor.byteOffset + indices_view.byteOffset;

	IndexType index_type = INDEX_TYPE_MAX;

	if (indices_accessor.componentType ==
			TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT) {
		index_type = INDEX_TYPE_UINT16;

		const uint32_t* indices_data = reinterpret_cast<const uint32_t*>(
				&indices_buffer.data[indices_offset]);
		indices.assign(indices_data, indices_data + indices_accessor.count);
	} else if (indices_accessor.componentType ==
			TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT) {
		index_type = INDEX_TYPE_UINT32;

		const uint32_t* indices_data = reinterpret_cast<const uint32_t*>(
				&indices_buffer.data[indices_offset]);
		indices.assign(indices_data, indices_data + indices_accessor.count);
	} else {
		GL_LOG_ERROR("Unsupported index component type: {}",
				indices_accessor.componentType);
		GL_ASSERT(false);
	}

	Ref<Mesh> new_mesh = Mesh::create(vertices, indices, index_type);
	new_mesh->model_index = p_model_index;
	new_mesh->name = p_name;

	if (p_primitive.material >= 0) {
		const auto& gltf_material = p_model.materials[p_primitive.material];

		// use pbr texture if available if not try to use normal texture
		new_mesh->diffuse_index =
				gltf_material.pbrMetallicRoughness.baseColorTexture.index;

		// select specular index based on metallicity
		// use color texture directly for non-metallic
		// materials and use normal (white) texture
		// otherwise
		new_mesh->specular_index = new_mesh->diffuse_index;

		MaterialConstants constants = {};
		constants.diffuse_factor = Color{
			(float)gltf_material.pbrMetallicRoughness.baseColorFactor[0],
			(float)gltf_material.pbrMetallicRoughness.baseColorFactor[1],
			(float)gltf_material.pbrMetallicRoughness.baseColorFactor[2],
			// it is not guaranteed this to be vec4
			gltf_material.pbrMetallicRoughness.baseColorFactor.size() > 3
					? (float)gltf_material.pbrMetallicRoughness
							  .baseColorFactor[3]
					: 1.0f,
		};

		constants.metallic_factor =
				gltf_material.pbrMetallicRoughness.metallicFactor;

		// TODO: dynamically set this from roughness factor.
		constants.shininess_factor = 64.0;

		MaterialResources resources = {};
		resources.constants = constants;
		resources.diffuse_image = _load_material_image(
				p_model, new_mesh->model_index, new_mesh->diffuse_index);
		resources.specular_image = _load_material_image(
				p_model, new_mesh->model_index, new_mesh->specular_index);

		// TODO get sampler from gltf
		resources.sampler = Renderer::get_default_sampler();

		new_mesh->material = p_material->create_instance(resources);
	}

	return new_mesh;
}

static void _process_node(const tinygltf::Model& p_model,
		const tinygltf::Node& p_node, const fs::path& p_directory,
		Ref<Material> p_material, Ref<Model> p_parent, int p_model_index) {
	if (p_node.mesh >= 0) {
		const auto& mesh = p_model.meshes[p_node.mesh];
		for (const auto& primitive : mesh.primitives) {
			p_parent->meshes.push_back(_process_mesh(p_model, primitive,
					p_directory, p_material, mesh.name, p_model_index));
		}
	}

	for (const auto& child_index : p_node.children) {
		const auto& child_node = p_model.nodes[child_index];
		_process_node(p_model, child_node, p_directory, p_material, p_parent,
				p_model_index);
	}
}

Ref<Model> Model::load(const fs::path& p_path, Ref<Material> p_material) {
	tinygltf::TinyGLTF loader;
	tinygltf::Model gltf;
	std::string err, warn;

	bool gltf_ret;
	if (p_path.extension() == ".glb") {
		gltf_ret =
				loader.LoadBinaryFromFile(&gltf, &err, &warn, p_path.string());
	} else if (p_path.extension() == ".gltf") {
		gltf_ret =
				loader.LoadASCIIFromFile(&gltf, &err, &warn, p_path.string());
	} else {
		GL_LOG_ERROR("Unknown file format!");
		return nullptr;
	}

	if (!warn.empty()) {
		GL_LOG_WARNING("Warning: {}", warn);
	}

	if (!gltf_ret) {
		GL_LOG_ERROR("Error: {}", err);
		return nullptr;
	}

	Ref<Model> model = create_ref<Model>();
	model->name = p_path.filename();

	static int s_mesh_counter = 0;

	for (const auto& node_index : gltf.scenes[gltf.defaultScene].nodes) {
		const auto& node = gltf.nodes[node_index];
		_process_node(gltf, node, p_path.parent_path(), p_material, model,
				s_mesh_counter++);
	}

	return model;
}

static void _destroy_loaded_image(
		uint32_t p_model_index, uint32_t p_texture_index) {
	size_t hash = _calculate_material_hash(p_model_index, p_texture_index);

	const auto it = s_loaded_images.find(hash);
	if (it == s_loaded_images.end()) {
		return;
	}

	LoadedImage& loaded_image = it->second;

	if (--loaded_image.usage_count < 1) {
		Renderer::get_backend()->image_free(loaded_image.image);
		s_loaded_images.erase(it);
	}
}

void Model::destroy(const Ref<Model> p_model) {
	for (auto mesh : p_model->meshes) {
		_destroy_loaded_image(mesh->model_index, mesh->diffuse_index);
		_destroy_loaded_image(mesh->model_index, mesh->specular_index);

		Mesh::destroy(mesh.get());
	}
}
