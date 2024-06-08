#include "renderer/mesh.h"

#include "renderer/render_backend.h"
#include "renderer/renderer.h"
#include "renderer/types.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>

struct LoadedImage {
	Image image;
	uint32_t usage_count;
};

static std::unordered_map<uint32_t, LoadedImage> s_loaded_images;

static Image _load_material_image(
		const tinygltf::Model& model, int texture_index) {
	if (texture_index < 0) {
		return nullptr;
	}

	const auto it = s_loaded_images.find(texture_index);
	if (it != s_loaded_images.end()) {
		LoadedImage& loaded_image = it->second;
		loaded_image.usage_count++;

		return loaded_image.image;
	}

	const auto& gltf_texture = model.textures[texture_index];
	const auto& gltf_image = model.images[gltf_texture.source];

	const Vec2u size = {
		static_cast<uint32_t>(gltf_image.height),
		static_cast<uint32_t>(gltf_image.width),
	};

	Ref<RenderBackend> backend = Renderer::get_backend();

	Image image = backend->image_create(
			DATA_FORMAT_R8G8B8A8_UNORM, size, (void*)gltf_image.image.data());

	s_loaded_images[texture_index] = LoadedImage{
		.image = image,
		.usage_count = 1,
	};

	return image;
}

static std::unordered_map<uint64_t, Sampler> s_loaded_samplers;

static Sampler _get_sampler(uint64_t p_id) {
	Ref<RenderBackend> backend = Renderer::get_backend();

	Sampler sampler = backend->sampler_create();
	s_loaded_samplers[p_id] = sampler;

	return sampler;
}

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

static Ref<Mesh> _process_mesh(const tinygltf::Model& p_model,
		const tinygltf::Primitive& p_primitive, const fs::path& p_directory,
		Ref<Material> p_material) {
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	// position accessor is guaranteed to exists but the others are not
	GLTFAccessor position_accessor =
			_get_gltf_accessor<float>(p_model, p_primitive, "POSITION").value();

	Optional<GLTFAccessor<float>> tex_coord_accessor =
			_get_gltf_accessor<float>(p_model, p_primitive, "TEXCOORD_0");
	Optional<GLTFAccessor<float>> normal_accessor =
			_get_gltf_accessor<float>(p_model, p_primitive, "NORMAL");

	static constexpr Vec2f DEFAULT_TEX_COORD = { 0.0f, 0.0f };
	static constexpr Vec3f DEFAULT_NORMAL = { 0.0f, 0.0f, 0.0f };

	for (size_t i = 0; i < position_accessor.accessor->count; ++i) {
		Vertex v;
		{
			memcpy(&v.position, position_accessor.get(i), sizeof(Vec3f));

			if (tex_coord_accessor.has_value()) {
				const float* tex_coord = tex_coord_accessor->get(i);
				v.uv_x = tex_coord[0];
				v.uv_y = tex_coord[1];
			} else {
				v.uv_x = DEFAULT_TEX_COORD.x;
				v.uv_y = DEFAULT_TEX_COORD.y;
			}

			if (normal_accessor.has_value()) {
				memcpy(&v.normal, normal_accessor->get(i), sizeof(Vec3f));
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
	const uint32_t* indices_data = reinterpret_cast<const uint32_t*>(
			&indices_buffer.data[indices_accessor.byteOffset +
					indices_view.byteOffset]);

	indices = std::vector<uint32_t>(
			indices_data, indices_data + indices_accessor.count);

	Ref<Mesh> new_mesh = Mesh::create(vertices, indices);

	if (p_primitive.material >= 0) {
		const auto& gltf_material = p_model.materials[p_primitive.material];

		new_mesh->color_index =
				gltf_material.pbrMetallicRoughness.baseColorTexture.index;
		new_mesh->roughness_index = gltf_material.pbrMetallicRoughness
											.metallicRoughnessTexture.index;

		Material::MaterialResources resources = {};
		resources.color_image =
				_load_material_image(p_model, new_mesh->color_index);
		resources.color_sampler = _get_sampler(new_mesh->vertex_buffer_address);

		new_mesh->material = p_material->create_instance(resources);
	}

	return new_mesh;
}

static void _process_node(const tinygltf::Model& p_model,
		const tinygltf::Node& p_node, const fs::path& p_directory,
		Ref<Material> p_material, Ref<Node> p_parent) {
	Ref<Node> base_node = create_ref<Node>();
	p_parent->add_child(base_node);

	if (p_node.mesh >= 0) {
		const auto& mesh = p_model.meshes[p_node.mesh];
		for (const auto& primitive : mesh.primitives) {
			base_node->add_child(
					_process_mesh(p_model, primitive, p_directory, p_material));
		}
	}

	for (const auto& child_index : p_node.children) {
		const auto& child_node = p_model.nodes[child_index];
		_process_node(p_model, child_node, p_directory, p_material, base_node);
	}
}

Ref<Node> Mesh::load(const fs::path& p_path, Ref<Material> p_material) {
	tinygltf::TinyGLTF loader;
	tinygltf::Model model;
	std::string err, warn;

	bool ret;

	if (p_path.extension() == ".glb") {
		ret = loader.LoadBinaryFromFile(&model, &err, &warn, p_path.string());
	} else if (p_path.extension() == ".gltf") {
		ret = loader.LoadASCIIFromFile(&model, &err, &warn, p_path.string());
	} else {
		GL_LOG_ERROR("Unknown file format!");
		return nullptr;
	}

	if (!warn.empty()) {
		GL_LOG_WARNING("Warning: {}", warn);
	}

	if (!ret) {
		GL_LOG_ERROR("Error: {}", err);
		return nullptr;
	}

	Ref<Node> root = create_ref<Node>();

	for (const auto& node_index : model.scenes[model.defaultScene].nodes) {
		const auto& node = model.nodes[node_index];
		_process_node(model, node, p_path.parent_path(), p_material, root);
	}

	return root;
}

Ref<Mesh> Mesh::create(
		std::span<Vertex> p_vertices, std::span<uint32_t> p_indices) {
	Ref<RenderBackend> backend = Renderer::get_backend();

	const uint32_t vertices_size = p_vertices.size() * sizeof(Vertex);
	const uint32_t indices_size = p_indices.size() * sizeof(uint32_t);

	Ref<Mesh> mesh = create_ref<Mesh>();
	mesh->index_count = p_indices.size();
	mesh->vertex_count = p_vertices.size();

	mesh->vertex_buffer = backend->buffer_create(vertices_size,
			BUFFER_USAGE_STORAGE_BUFFER_BIT | BUFFER_USAGE_TRANSFER_DST_BIT |
					BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			MEMORY_ALLOCATION_TYPE_GPU);

	// get the address
	mesh->vertex_buffer_address =
			backend->buffer_get_device_address(mesh->vertex_buffer);

	mesh->index_buffer = backend->buffer_create(indices_size,
			BUFFER_USAGE_INDEX_BUFFER_BIT | BUFFER_USAGE_TRANSFER_DST_BIT,
			MEMORY_ALLOCATION_TYPE_GPU);

	Buffer staging_buffer = backend->buffer_create(vertices_size + indices_size,
			BUFFER_USAGE_TRANSFER_SRC_BIT, MEMORY_ALLOCATION_TYPE_CPU);

	void* data = backend->buffer_map(staging_buffer);
	{
		// copy vertex data
		memcpy(data, p_vertices.data(), vertices_size);
		// copy index data
		memcpy((uint8_t*)data + vertices_size, p_indices.data(), indices_size);
	}
	backend->buffer_unmap(staging_buffer);

	backend->command_immediate_submit([&](CommandBuffer p_cmd) {
		BufferCopyRegion vertex_copy = {};
		vertex_copy.src_offset = 0;
		vertex_copy.dst_offset = 0;
		vertex_copy.size = vertices_size;

		backend->command_copy_buffer(
				p_cmd, staging_buffer, mesh->vertex_buffer, vertex_copy);

		BufferCopyRegion index_copy = {};
		index_copy.src_offset = vertices_size;
		index_copy.dst_offset = 0;
		index_copy.size = indices_size;

		backend->command_copy_buffer(
				p_cmd, staging_buffer, mesh->index_buffer, index_copy);
	});

	backend->buffer_free(staging_buffer);

	return mesh;
}

static void _destroy_loaded_image(const uint32_t p_index) {
	Ref<RenderBackend> backend = Renderer::get_backend();

	const auto it = s_loaded_images.find(p_index);
	if (it == s_loaded_images.end()) {
		return;
	}

	LoadedImage& loaded_image = it->second;

	if (--loaded_image.usage_count < 1) {
		backend->image_free(loaded_image.image);
		s_loaded_images.erase(it);
	}
}

void Mesh::destroy(const Mesh* p_mesh) {
	Ref<RenderBackend> backend = Renderer::get_backend();

	_destroy_loaded_image(p_mesh->color_index);

	backend->sampler_free(s_loaded_samplers[p_mesh->vertex_buffer_address]);

	backend->buffer_free(p_mesh->vertex_buffer);
	backend->buffer_free(p_mesh->index_buffer);
}
