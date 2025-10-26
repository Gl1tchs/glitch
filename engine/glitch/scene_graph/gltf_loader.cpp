#include "glitch/scene_graph/gltf_loader.h"

#include "glitch/core/application.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>

namespace gl {

enum GLTFParsingFlags : uint16_t {
	GLTF_PARSING_FLAG_NO_UV = 0x1,
	GLTF_PARSING_FLAG_NO_NORMALS = 0x2,
};

GLTFLoader::GLTFLoader() {
	default_texture = Texture::create(COLOR_WHITE, { 1, 1 });

	default_material = MaterialSystem::create_instance("urp_standart");
	default_material->set_param("base_color", COLOR_WHITE);
	default_material->set_param("metallic", 0.5f);
	default_material->set_param("roughness", 0.5f);
	default_material->set_param("u_diffuse_texture", default_texture);
	default_material->set_param("u_normal_texture", default_texture);
	default_material->set_param(
			"u_metallic_roughness_texture", default_texture);
	default_material->set_param("u_ambient_occlusion_texture", default_texture);
	default_material->upload();
}

GLTFLoader::~GLTFLoader() {
	// Wait for device to finish operations before destructing materials
	Application::get_render_backend()->device_wait();

	loaded_textures.clear();
}

static size_t _hash_gltf_model(const tinygltf::Model& p_model) {
	size_t seed = 0;

	for (const auto& buffer : p_model.buffers) {
		if (!buffer.data.empty()) {
			hash_combine(seed,
					hash64(buffer.data.data(),
							std::min<size_t>(buffer.data.size(), 1024)));
		}
	}

	for (const auto& mesh : p_model.meshes) {
		for (const auto& primitive : mesh.primitives) {
			// Hash the attribute keys and indices count
			for (const auto& attr : primitive.attributes) {
				hash_combine(seed, hash64(attr.first));
				hash_combine(seed, hash64(attr.second));
			}
			hash_combine(seed, hash64(primitive.indices));
		}
	}

	for (const auto& material : p_model.materials) {
		hash_combine(seed, hash64(material.name));
		hash_combine(
				seed, hash64(material.pbrMetallicRoughness.baseColorFactor));
		hash_combine(
				seed, hash64(material.pbrMetallicRoughness.metallicFactor));
		hash_combine(
				seed, hash64(material.pbrMetallicRoughness.roughnessFactor));
	}

	for (const auto& image : p_model.images) {
		hash_combine(seed, hash64(image.name));
		hash_combine(seed, hash64(image.image.size()));
	}

	return seed;
}

Result<Ref<SceneNode>, std::string> GLTFLoader::load_gltf(
		const fs::path& p_path, Ref<MaterialInstance> p_overload_material) {
	// TODO: better validation
	if (!p_path.has_extension() ||
			!(p_path.extension() == ".glb" || p_path.extension() == ".gltf")) {
		return make_err<Ref<SceneNode>>(
				std::string("Unable to parse non gltf formats."));
	}

	tinygltf::Model model;
	tinygltf::TinyGLTF loader;
	std::string err, warn;

	bool ret;
	if (p_path.extension() == ".glb") {
		ret = loader.LoadBinaryFromFile(&model, &err, &warn, p_path.string());
	} else {
		ret = loader.LoadASCIIFromFile(&model, &err, &warn, p_path.string());
	}

	if (!ret) {
		return make_err<Ref<SceneNode>>(err);
	}

	Ref<SceneNode> base_node = create_ref<SceneNode>();

	const size_t model_hash = _hash_gltf_model(model);
	const fs::path base_path = p_path.parent_path();

	const tinygltf::Scene& scene = model.scenes[model.defaultScene];
	for (int node_index : scene.nodes) {
		_parse_node(node_index, &model, model_hash, base_path,
				p_overload_material, base_node);
	}

	return base_node;
}

Future<Result<Ref<SceneNode>, std::string>> GLTFLoader::load_gltf_async(
		const fs::path& p_path, Ref<MaterialInstance> p_overload_material) {
	return Future<Result<Ref<SceneNode>, std::string>>::async(
			[this, p_path, p_overload_material]() {
				return load_gltf(p_path, p_overload_material);
			});
}

void GLTFLoader::_parse_node(int p_node_idx, const tinygltf::Model* p_model,
		const size_t p_model_hash, const fs::path& p_base_path,
		Ref<MaterialInstance> p_overload_material,
		Ref<SceneNode> p_parent_node) {
	Ref<SceneNode> node = create_ref<SceneNode>();
	if (p_parent_node) {
		p_parent_node->add_child(node);
	}

	// Create and load mesh primitive if exists
	const tinygltf::Node& gltf_node = p_model->nodes[p_node_idx];
	if (gltf_node.mesh >= 0) {
		node->mesh = _load_mesh(&gltf_node, p_model, p_model_hash, p_base_path,
				p_overload_material);
	}

	// Parse translation
	if (gltf_node.matrix.size() == 16) {
		// Use matrix
		glm::mat4 mat = glm::make_mat4(gltf_node.matrix.data());

		glm::vec3 skew;
		glm::vec4 perspective;
		glm::fquat rotation_quat;

		glm::decompose(mat, node->transform.scale, rotation_quat,
				node->transform.position, skew, perspective);

		node->transform.rotation =
				glm::degrees(glm::eulerAngles(rotation_quat));
	} else {
		// Use TRS

		if (gltf_node.translation.size() == 3) {
			node->transform.position = glm::vec3(gltf_node.translation[0],
					gltf_node.translation[1], gltf_node.translation[2]);
		}

		if (gltf_node.rotation.size() == 4) {
			node->transform.rotation = glm::degrees(glm::eulerAngles(
					glm::fquat(gltf_node.rotation[3], gltf_node.rotation[0],
							gltf_node.rotation[1], gltf_node.rotation[2])));
		}

		if (gltf_node.scale.size() == 3) {
			node->transform.scale = glm::vec3(
					gltf_node.scale[0], gltf_node.scale[1], gltf_node.scale[2]);
		}
	}

	for (int child_node_idx : gltf_node.children) {
		_parse_node(child_node_idx, p_model, -p_model_hash, p_base_path,
				p_overload_material, node);
	}
}

Ref<Mesh> GLTFLoader::_load_mesh(const tinygltf::Node* p_gltf_node,
		const tinygltf::Model* p_model, const size_t p_model_hash,
		const fs::path& p_base_path,
		Ref<MaterialInstance> p_overload_material) {
	Ref<Mesh> mesh = create_ref<Mesh>();

	const tinygltf::Mesh& gltf_mesh = p_model->meshes[p_gltf_node->mesh];
	for (const auto& primitive : gltf_mesh.primitives) {
		mesh->primitives.push_back(_load_primitive(&primitive, p_model,
				p_model_hash, &gltf_mesh, p_base_path, p_overload_material));
	}

	return mesh;
}

static ImageFiltering _gltf_to_image_filtering(int p_gltf_filter) {
	switch (p_gltf_filter) {
		case TINYGLTF_TEXTURE_FILTER_NEAREST:
		case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
		case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR:
			return ImageFiltering::NEAREST;
		case TINYGLTF_TEXTURE_FILTER_LINEAR:
		case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
		case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:
			return ImageFiltering::LINEAR;
		default:
			return ImageFiltering::LINEAR;
	}
}

static ImageWrappingMode _gltf_to_image_wrapping(int p_gltf_wrap) {
	switch (p_gltf_wrap) {
		case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE:
			return ImageWrappingMode::CLAMP_TO_EDGE;
		case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT:
			return ImageWrappingMode::MIRRORED_REPEAT;
		case TINYGLTF_TEXTURE_WRAP_REPEAT:
			return ImageWrappingMode::REPEAT;
		default:
			return ImageWrappingMode::CLAMP_TO_EDGE;
	}
}

static int _get_extension_texture_index(
		const tinygltf::Value& p_extension, const std::string& p_field_name) {
	if (p_extension.Has(p_field_name)) {
		const auto& texture_value = p_extension.Get(p_field_name);
		if (texture_value.IsObject() && texture_value.Has("index")) {
			int texture_index = texture_value.Get("index").Get<int>();
			if (texture_index >= 0) {
				return texture_index;
			}
		}
	}
	return -1;
}

Ref<MeshPrimitive> GLTFLoader::_load_primitive(
		const tinygltf::Primitive* p_primitive, const tinygltf::Model* p_model,
		const size_t p_model_hash, const tinygltf::Mesh* p_mesh,
		const fs::path& p_base_path,
		Ref<MaterialInstance> p_overload_material) {
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

	// Create material instance if no material for overload provided
	if (!p_overload_material && p_primitive->material >= 0 &&
			p_primitive->material < p_model->materials.size()) {
		const tinygltf::Material& gltf_material =
				p_model->materials[p_primitive->material];
		const auto& base_color =
				gltf_material.pbrMetallicRoughness.baseColorFactor;

		material = MaterialSystem::create_instance("urp_standart");

		material->set_param("base_color",
				Color(base_color[0], base_color[1], base_color[2],
						base_color[3]));
		material->set_param("metallic",
				static_cast<float>(
						gltf_material.pbrMetallicRoughness.metallicFactor));
		material->set_param("roughness",
				static_cast<float>(
						gltf_material.pbrMetallicRoughness.roughnessFactor));

		// Check for pbr specular glossines extention first and use that if
		// exists
		if (const auto it = gltf_material.extensions.find(
					"KHR_materials_pbrSpecularGlossiness");
				it != gltf_material.extensions.end()) {
			const tinygltf::Value& specGloss = it->second;

			// DiffuseFactor
			const auto& diffuse_factor = specGloss.Has("diffuseFactor")
					? specGloss.Get("diffuseFactor")
							  .Get<tinygltf::Value::Array>()
					: tinygltf::Value::Array{ tinygltf::Value(1.0),
						  tinygltf::Value(1.0), tinygltf::Value(1.0),
						  tinygltf::Value(1.0) };

			material->set_param("base_color",
					Color(float(diffuse_factor[0].GetNumberAsDouble()),
							float(diffuse_factor[1].GetNumberAsDouble()),
							float(diffuse_factor[2].GetNumberAsDouble()),
							float(diffuse_factor[3].GetNumberAsDouble())));

			// Load diffuseTexture → u_diffuse_texture
			int diffuse_texture_index =
					_get_extension_texture_index(specGloss, "diffuseTexture");
			Ref<Texture> diffuse_texture = (diffuse_texture_index >= 0)
					? _load_texture(diffuse_texture_index, p_model,
							  p_model_hash, p_base_path)
					: default_texture;
			material->set_param("u_diffuse_texture", diffuse_texture);

			// Load specularGlossinessTexture → u_specular_texture
			int specular_texture_index = _get_extension_texture_index(
					specGloss, "specularGlossinessTexture");
			Ref<Texture> specular_texture = (specular_texture_index >= 0)
					? _load_texture(specular_texture_index, p_model,
							  p_model_hash, p_base_path)
					: default_texture;
			material->set_param(
					"u_metallic_roughness_texture", specular_texture);

			// Metallic / Roughness are not used in SpecGloss → set defaults
			material->set_param("metallic", 0.0f);
			material->set_param("roughness", 1.0f);
		} else {
			// Standard PBR flow
			const auto& base_color =
					gltf_material.pbrMetallicRoughness.baseColorFactor;
			material->set_param("base_color",
					Color(base_color[0], base_color[1], base_color[2],
							base_color[3]));

			material->set_param("metallic",
					static_cast<float>(
							gltf_material.pbrMetallicRoughness.metallicFactor));
			material->set_param("roughness",
					static_cast<float>(gltf_material.pbrMetallicRoughness
									.roughnessFactor));

			// Load baseColorTexture → u_diffuse_texture
			int albedo_texture_index =
					gltf_material.pbrMetallicRoughness.baseColorTexture.index;
			Ref<Texture> albedo_texture = (albedo_texture_index >= 0)
					? _load_texture(albedo_texture_index, p_model, p_model_hash,
							  p_base_path)
					: default_texture;
			material->set_param("u_diffuse_texture", albedo_texture);

			// Load metallic-roughness texture
			int metallic_roughness_texture_index =
					gltf_material.pbrMetallicRoughness.metallicRoughnessTexture
							.index;
			Ref<Texture> metallic_roughness_texture =
					(metallic_roughness_texture_index >= 0)
					? _load_texture(metallic_roughness_texture_index, p_model,
							  p_model_hash, p_base_path)
					: default_texture;
			material->set_param(
					"u_metallic_roughness_texture", metallic_roughness_texture);
		}

		{
			const int normal_texture_index = gltf_material.normalTexture.index;
			Ref<Texture> normal_texture = (normal_texture_index > 0)
					? _load_texture(normal_texture_index, p_model, p_model_hash,
							  p_base_path)
					: default_texture;
			material->set_param("u_normal_texture", normal_texture);
		}

		{
			const int occlusion_texture_index =
					gltf_material.occlusionTexture.index;
			Ref<Texture> occlusion_texture = (occlusion_texture_index > 0)
					? _load_texture(occlusion_texture_index, p_model,
							  p_model_hash, p_base_path)
					: default_texture;
			material->set_param(
					"u_ambient_occlusion_texture", occlusion_texture);
		}

		material->upload();
	}

	Ref<MeshPrimitive> prim =
			MeshPrimitive::create(prim_vertices, prim_indices);
	prim->material = !p_overload_material ? material : p_overload_material;

	return prim;
}

Ref<Texture> GLTFLoader::_load_texture(int texture_index,
		const tinygltf::Model* p_model, const size_t p_model_hash,
		const fs::path& p_base_path) {
	size_t hash = 0;
	hash_combine(hash, texture_index);
	hash_combine(hash, p_model_hash);

	if (loaded_textures.find(hash) != loaded_textures.end()) {
		// Texture already loaded
		return loaded_textures.at(hash);
	}

	// Texture not loaded
	const tinygltf::Texture& gltf_texture = p_model->textures[texture_index];
	const tinygltf::Image& gltf_image = p_model->images[gltf_texture.source];

	// Parse sampler
	TextureSamplerOptions sampler_options = {};
	if (gltf_texture.sampler >= 0) {
		const tinygltf::Sampler& sampler =
				p_model->samplers[gltf_texture.sampler];

		// Texture filtering
		sampler_options.mag_filter =
				_gltf_to_image_filtering(sampler.magFilter);
		sampler_options.min_filter =
				_gltf_to_image_filtering(sampler.minFilter);

		// Texture wrapping
		sampler_options.wrap_u = _gltf_to_image_wrapping(sampler.wrapS);
		sampler_options.wrap_v = _gltf_to_image_wrapping(sampler.wrapT);
	}

	Ref<Texture> texture;
	if (gltf_image.uri.empty()) {
		// Embedded — create directly from buffer

		DataFormat format;
		switch (gltf_image.component) {
			case 1:
				format = DataFormat::R8_UNORM;
				break;
			case 2:
				format = DataFormat::R8G8_UNORM;
				break;
			case 3:
				format = DataFormat::R8G8B8_UNORM;
				break;
			case 4:
				format = DataFormat::R8G8B8A8_UNORM;
				break;
			default:
				GL_ASSERT(false,
						"Unsupported image component "
						"count");
		}

		texture = Texture::create(format,
				glm::uvec2(gltf_image.width, gltf_image.height),
				gltf_image.image.data(), sampler_options);
	} else {
		// External file — use loader
		texture = Texture::load_from_path(
				p_base_path / gltf_image.uri, sampler_options);
	}

	loaded_textures[hash] = texture;

	return texture;
}

} //namespace gl