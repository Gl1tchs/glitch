#include "gl/renderer/mesh.h"

#include "gl/renderer/image.h"
#include "gl/renderer/material.h"
#include "gl/renderer/renderer.h"

#include "platform/vulkan/vk_mesh.h"
#include "platform/vulkan/vk_renderer.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>

struct LoadedImage {
	Ref<Image> image;
	uint32_t usage_count;
};
static std::unordered_map<uint32_t, LoadedImage> s_loaded_images;

static Ref<Image> load_material_image(
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

	ImageCreateInfo info = {
		.format = ImageFormat::R8G8B8A8_UNORM,
		.size = { 
            static_cast<uint32_t>(gltf_image.height),
            static_cast<uint32_t>(gltf_image.width),
        },
		.data = (void*)gltf_image.image.data(),
		.mipmapped = false,
	};
	Ref<Image> image = Image::create(&info);

	s_loaded_images[texture_index] = LoadedImage{
		.image = image,
		.usage_count = 1,
	};

	return image;
}

template <typename T> struct GLTFAccessor {
	const tinygltf::Accessor* accessor;
	const T* data;

	inline const T* get(const int idx) {
		return reinterpret_cast<const T*>(data + (idx * accessor->type));
	}
};

template <typename T>
static const T* get_gltf_accessor_data(
		const tinygltf::Model& model, const tinygltf::Accessor* accessor) {
	const tinygltf::BufferView& view = model.bufferViews[accessor->bufferView];
	const tinygltf::Buffer& buffer = model.buffers[view.buffer];

	const T* data = reinterpret_cast<const T*>(
			&buffer.data[accessor->byteOffset + view.byteOffset]);
	return data;
}

template <typename T>
static Optional<GLTFAccessor<T>> get_gltf_accessor(const tinygltf::Model& model,
		const tinygltf::Primitive& primitive, const std::string& name) {
	const bool accessor_exists =
			primitive.attributes.find(name) != primitive.attributes.end();

	if (!accessor_exists) {
		return {};
	}

	const auto& gltf_accessor = model.accessors[primitive.attributes.at(name)];
	const auto* gltf_data =
			get_gltf_accessor_data<float>(model, &gltf_accessor);

	GLTFAccessor accessor = {
		.accessor = &gltf_accessor,
		.data = gltf_data,
	};

	return accessor;
}

static Ref<Mesh> process_mesh(const tinygltf::Model& model,
		const tinygltf::Primitive& primitive, const fs::path& directory,
		Ref<MetallicRoughnessMaterial> material) {
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	// position accessor is guaranteed to exists but the others are not
	GLTFAccessor position_accessor =
			get_gltf_accessor<float>(model, primitive, "POSITION").value();

	Optional<GLTFAccessor<float>> tex_coord_accessor =
			get_gltf_accessor<float>(model, primitive, "TEXCOORD_0");
	Optional<GLTFAccessor<float>> normal_accessor =
			get_gltf_accessor<float>(model, primitive, "NORMAL");

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
			model.accessors[primitive.indices];
	const tinygltf::BufferView& indices_view =
			model.bufferViews[indices_accessor.bufferView];

	const tinygltf::Buffer& indices_buffer = model.buffers[indices_view.buffer];
	const uint32_t* indices_data = reinterpret_cast<const uint32_t*>(
			&indices_buffer.data[indices_accessor.byteOffset +
					indices_view.byteOffset]);

	indices = std::vector<uint32_t>(
			indices_data, indices_data + indices_accessor.count);

	Ref<Mesh> new_mesh = Mesh::create(vertices, indices);

	if (primitive.material >= 0) {
		const auto& gltf_material = model.materials[primitive.material];

		new_mesh->color_index =
				gltf_material.pbrMetallicRoughness.baseColorTexture.index;
		new_mesh->roughness_index = gltf_material.pbrMetallicRoughness
											.metallicRoughnessTexture.index;

		MetallicRoughnessMaterial::MaterialConstants constants = {
			.color_factors = Vec4f(1.0f),
			.metal_rough_factors = Vec4f(1.0f),
		};

		MetallicRoughnessMaterial::MaterialResources resources = {
			.constants = constants,
			.constants_offset = 0,
			.color_image = load_material_image(model, new_mesh->color_index),
			.color_filtering = ImageFilteringMode::LINEAR,
			.roughness_image =
					load_material_image(model, new_mesh->roughness_index),
			.roughness_filtering = ImageFilteringMode::LINEAR,
		};
		new_mesh->material = material->create_instance(resources);
	}

	return new_mesh;
}

void process_node(const tinygltf::Model& model, const tinygltf::Node& node,
		const fs::path& directory, Ref<MetallicRoughnessMaterial> material,
		Ref<Node> parent) {
	Ref<Node> base_node = create_ref<Node>();
	parent->add_child(base_node);

	if (node.mesh >= 0) {
		const auto& mesh = model.meshes[node.mesh];
		for (const auto& primitive : mesh.primitives) {
			base_node->add_child(
					process_mesh(model, primitive, directory, material));
		}
	}

	for (const auto& child_index : node.children) {
		const auto& child_node = model.nodes[child_index];
		process_node(model, child_node, directory, material, base_node);
	}
}

Ref<Node> Mesh::load(
		const fs::path& path, Ref<MetallicRoughnessMaterial> material) {
	tinygltf::TinyGLTF loader;
	tinygltf::Model model;
	std::string err, warn;

	bool ret;

	if (path.extension() == ".glb") {
		ret = loader.LoadBinaryFromFile(&model, &err, &warn, path.string());
	} else if (path.extension() == ".gltf") {
		ret = loader.LoadASCIIFromFile(&model, &err, &warn, path.string());
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
		process_node(model, node, path.parent_path(), material, root);
	}

	return root;
}

Ref<Mesh> Mesh::create(
		const std::span<Vertex> vertices, const std::span<uint32_t> indices) {
	switch (Renderer::get_backend()) {
		case RenderBackend::Vulkan: {
			Ref<VulkanMesh> vk_mesh = VulkanMesh::create(
					VulkanRenderer::get_context(), vertices, indices);
			vk_mesh->index_count = indices.size();
			vk_mesh->vertex_count = vertices.size();
			return vk_mesh;
		}

		default: {
			return nullptr;
		}
	}
}

static void destroy_loaded_image(const uint32_t index) {
	const auto it = s_loaded_images.find(index);
	if (it == s_loaded_images.end()) {
		return;
	}

	LoadedImage& loaded_image = it->second;

	if (--loaded_image.usage_count < 1) {
		Image::destroy(loaded_image.image);
		s_loaded_images.erase(it);
	}
}

void Mesh::destroy(Mesh* mesh) {
	if (mesh->destroyed) {
		return;
	}

	switch (Renderer::get_backend()) {
		case RenderBackend::Vulkan: {
			destroy_loaded_image(mesh->color_index);
			destroy_loaded_image(mesh->roughness_index);

			VulkanMesh* vk_mesh = reinterpret_cast<VulkanMesh*>(mesh);
			VulkanMesh::destroy(VulkanRenderer::get_context(), vk_mesh);

			mesh->destroyed = true;
			break;
		}
		default: {
			break;
		}
	}
}
