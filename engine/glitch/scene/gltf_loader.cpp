#include "glitch/scene/gltf_loader.h"

#include "glitch/asset/asset_system.h"
#include "glitch/renderer/material.h"
#include "glitch/renderer/mesh.h"
#include "glitch/renderer/texture.h"
#include "glitch/scene/components.h"
#include "glitch/scene/scene_renderer.h"

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

struct GLTFLoadContext {
	std::shared_ptr<Scene> scene;
	const tinygltf::Model* model;
	size_t model_hash;
	fs::path base_path;
	UID model_id;
	std::unordered_map<size_t, AssetHandle> loaded_textures;
	std::unordered_map<int, AssetHandle> loaded_materials;
};

static size_t _hash_gltf_model(const tinygltf::Model& p_model);

static void _parse_gltf_node(GLTFLoadContext& p_ctx, int p_node_idx, Entity p_parent);

static std::shared_ptr<StaticMesh> _load_static_mesh(const tinygltf::Primitive* p_primitive,
		const tinygltf::Mesh* p_mesh, GLTFLoadContext& p_ctx);

static AssetHandle _load_material(int material_index, GLTFLoadContext& p_ctx);

static AssetHandle _load_texture(int texture_index, GLTFLoadContext& p_ctx);

static AssetHandle s_default_texture = INVALID_ASSET_HANDLE;
static AssetHandle s_default_material = INVALID_ASSET_HANDLE;

GLTFLoadError GLTFLoader::load(std::shared_ptr<Scene> p_scene, const std::string& p_path) {
	const auto abs_path_result = AssetSystem::get_absolute_path(p_path);
	if (!abs_path_result) {
		GL_LOG_ERROR("[GLTFLoader::load] Unable to parse relative format.");
		return GLTFLoadError::PATH_ERROR;
	}

	const fs::path abs_path = abs_path_result.get_value();

	// TODO: better validation
	if (!abs_path.has_extension() ||
			!(abs_path.extension() == ".glb" || abs_path.extension() == ".gltf")) {
		GL_LOG_ERROR("[GLTFLoader::load] Unable to parse non gltf formats.");
		return GLTFLoadError::INVALID_EXTENSION;
	}

	tinygltf::Model model;
	tinygltf::TinyGLTF loader;
	std::string err, warn;

	bool ret;
	if (abs_path.extension() == ".glb") {
		ret = loader.LoadBinaryFromFile(&model, &err, &warn, abs_path.string());
	} else {
		ret = loader.LoadASCIIFromFile(&model, &err, &warn, abs_path.string());
	}

	if (!ret) {
		GL_LOG_ERROR("[GLTFLoader::load] Unable to parse GLTF file.");
		if (!err.empty()) {
			GL_LOG_ERROR("[GLTFLoader::load] [GLTF]:\n%s", err);
		}
		return GLTFLoadError::PARSING_ERROR;
	}

#ifdef GL_DEBUG_BUILD
	GL_LOG_TRACE("[GLTFLoader::load_gltf] Loading GLTF Model from path '{}'", abs_path.string());

	if (!warn.empty()) {
		GL_LOG_WARNING("[GLTFLoader::load] [GLTF]:\n%s", warn);
	}

	if (!err.empty()) {
		GL_LOG_ERROR("[GLTFLoader::load] [GLTF]:\n%s", err);
	}
#endif

	Entity base_entity = p_scene->create(abs_path.filename().string());
	// Add GLTFSourceComponent for scene (de)serialization
	const GLTFSourceComponent* gltf_sc =
			base_entity.add_component<GLTFSourceComponent>(UID(), p_path);

	GLTFLoadContext ctx;
	ctx.scene = p_scene;
	ctx.model = &model;
	ctx.model_hash = _hash_gltf_model(model);
	ctx.base_path = abs_path.parent_path();
	ctx.model_id = gltf_sc->model_id;

	// Lazy initialization of defaults
	if (!s_default_texture || !AssetSystem::get<Texture>(s_default_texture)) {
		auto tex = Texture::create(COLOR_WHITE, { 1, 1 });
		s_default_texture = AssetSystem::register_asset(tex);
	}
	if (!s_default_material || !AssetSystem::get<Material>(s_default_material)) {
		auto mat = Material::create(DEFINITION_PATH_PBR_STANDARD);
		mat->set_param("base_color", VEC3_ONE);
		mat->set_param("metallic", 0.5f);
		mat->set_param("roughness", 0.5f);
		mat->set_param("u_diffuse_texture", s_default_texture);
		mat->set_param("u_normal_texture", s_default_texture);
		mat->set_param("u_metallic_roughness_texture", s_default_texture);
		mat->set_param("u_ambient_occlusion_texture", s_default_texture);
		mat->upload();

		s_default_material = AssetSystem::register_asset(mat);
	}

	for (int node_index : model.scenes[model.defaultScene].nodes) {
		_parse_gltf_node(ctx, node_index, base_entity);
	}

	return GLTFLoadError::NONE;
}

void _parse_gltf_node(GLTFLoadContext& p_ctx, int p_node_idx, Entity p_parent) {
	const tinygltf::Node& gltf_node = p_ctx.model->nodes[p_node_idx];

	Entity entity = p_ctx.scene->create(gltf_node.name, p_parent);

	if (gltf_node.matrix.size() == 16) {
		glm::mat4 mat = glm::make_mat4(gltf_node.matrix.data());

		glm::vec3 skew;
		glm::vec4 perspective;
		glm::fquat rotation_quat;

		glm::decompose(mat, entity.get_transform().local_scale, rotation_quat,
				entity.get_transform().local_position, skew, perspective);

		entity.get_transform().get_rotation() = glm::degrees(glm::eulerAngles(rotation_quat));
	} else {
		if (gltf_node.translation.size() == 3) {
			entity.get_transform().local_position = glm::vec3(
					gltf_node.translation[0], gltf_node.translation[1], gltf_node.translation[2]);
		}

		if (gltf_node.rotation.size() == 4) {
			entity.get_transform().local_rotation =
					glm::degrees(glm::eulerAngles(glm::fquat(gltf_node.rotation[3],
							gltf_node.rotation[0], gltf_node.rotation[1], gltf_node.rotation[2])));
		}

		if (gltf_node.scale.size() == 3) {
			entity.get_transform().local_scale =
					glm::vec3(gltf_node.scale[0], gltf_node.scale[1], gltf_node.scale[2]);
		}
	}

	// Load mesh
	if (gltf_node.mesh >= 0) {
		entity.add_component<GLTFInstanceComponent>(p_ctx.model_id, gltf_node.mesh);

		const tinygltf::Mesh& gltf_mesh = p_ctx.model->meshes[gltf_node.mesh];

		// Lambda to attach components to an entity
		const auto attach_mesh_components =
				[&](Entity target_entity, const tinygltf::Primitive& primitive, int prim_index) {
					// Load Geometry
					const std::shared_ptr<StaticMesh> static_mesh =
							_load_static_mesh(&primitive, &gltf_mesh, p_ctx);

					MeshComponent* mc = target_entity.add_component<MeshComponent>();
					// Register unique mesh asset
					mc->mesh = AssetSystem::register_asset(static_mesh,
							std::format("mem://Mesh/GLTF/?node={}&&prim={}&&model={}",
									p_ctx.model_hash, gltf_node.name, prim_index));
					mc->visible = true;

					// Load/Attach Material
					MaterialComponent* mat_comp = target_entity.add_component<MaterialComponent>();
					mat_comp->definition_path = DEFINITION_PATH_PBR_STANDARD;
					mat_comp->handle = _load_material(primitive.material, p_ctx);
				};

		// If single primitive, attach to the main Node entity
		if (gltf_mesh.primitives.size() == 1) {
			attach_mesh_components(entity, gltf_mesh.primitives[0], 0);
		}
		// If multiple primitives, create sub-entities
		else {
			for (size_t i = 0; i < gltf_mesh.primitives.size(); ++i) {
				Entity prim_entity =
						p_ctx.scene->create(std::format("{}_prim_{}", gltf_node.name, i), entity);
				attach_mesh_components(prim_entity, gltf_mesh.primitives[i], i);
			}
		}
	}

	for (int child_node_idx : gltf_node.children) {
		_parse_gltf_node(p_ctx, child_node_idx, entity);
	}
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

std::shared_ptr<StaticMesh> _load_static_mesh(const tinygltf::Primitive* p_primitive,
		const tinygltf::Mesh* p_mesh, GLTFLoadContext& p_ctx) {
	uint16_t parsing_flags = 0;

	const auto& pos_accessor = p_ctx.model->accessors[p_primitive->attributes.at("POSITION")];
	const auto& pos_view = p_ctx.model->bufferViews[pos_accessor.bufferView];
	const auto& pos_buffer = p_ctx.model->buffers[pos_view.buffer];

	const auto& index_accessor = p_ctx.model->accessors[p_primitive->indices];
	const auto& index_view = p_ctx.model->bufferViews[index_accessor.bufferView];
	const auto& index_buffer = p_ctx.model->buffers[index_view.buffer];

	uint32_t uv_accessor_offset;
	uint32_t uv_view_offset;
	const tinygltf::Buffer* uv_buffer = nullptr;
	if (p_primitive->attributes.find("TEXCOORD_0") != p_primitive->attributes.end()) {
		const auto& uv_accessor = p_ctx.model->accessors[p_primitive->attributes.at("TEXCOORD_0")];
		const auto& uv_view = p_ctx.model->bufferViews[uv_accessor.bufferView];

		uv_accessor_offset = uv_accessor.byteOffset;
		uv_view_offset = uv_view.byteOffset;
		uv_buffer = &p_ctx.model->buffers[uv_view.buffer];
	} else {
		parsing_flags |= GLTF_PARSING_FLAG_NO_UV;
	}

	uint32_t normal_accessor_offset;
	uint32_t normal_view_offset;
	const tinygltf::Buffer* normal_buffer = nullptr;
	if (p_primitive->attributes.find("NORMAL") != p_primitive->attributes.end()) {
		const auto& normal_accessor = p_ctx.model->accessors[p_primitive->attributes.at("NORMAL")];
		const auto& normal_view = p_ctx.model->bufferViews[normal_accessor.bufferView];

		normal_accessor_offset = normal_accessor.byteOffset;
		normal_view_offset = normal_view.byteOffset;
		normal_buffer = &p_ctx.model->buffers[normal_view.buffer];
	} else {
		parsing_flags |= GLTF_PARSING_FLAG_NO_NORMALS;
	}

	const size_t vertex_count = pos_accessor.count;
	std::vector<MeshVertex> prim_vertices(vertex_count);

	constexpr float DEFAULT_NORMAL_DATA[3] = { 0, 0, 0 };
	constexpr float DEFAULT_UV_DATA[3] = { 0, 0 };

	for (size_t i = 0; i < vertex_count; ++i) {
		const float* pos = reinterpret_cast<const float*>(
				&pos_buffer.data[pos_view.byteOffset + pos_accessor.byteOffset + i * 12]);

		const float* uv = (parsing_flags & GLTF_PARSING_FLAG_NO_UV)
				? DEFAULT_UV_DATA
				: reinterpret_cast<const float*>(
						  &uv_buffer->data[uv_view_offset + uv_accessor_offset + i * 8]);

		const float* normal = (parsing_flags & GLTF_PARSING_FLAG_NO_NORMALS)
				? DEFAULT_NORMAL_DATA
				: reinterpret_cast<const float*>(&normal_buffer
								  ->data[normal_view_offset + normal_accessor_offset + i * 12]);

		prim_vertices[i] = {
			glm::vec3(pos[0], pos[1], pos[2]),
			uv[0],
			glm::vec3(normal[0], normal[1], normal[2]),
			uv[1],
		};
	}

	const size_t index_count = index_accessor.count;
	std::vector<uint32_t> prim_indices(index_count);

	switch (index_accessor.componentType) {
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
			const uint16_t* indices = reinterpret_cast<const uint16_t*>(
					&index_buffer.data[index_view.byteOffset + index_accessor.byteOffset]);
			for (size_t i = 0; i < index_count; ++i) {
				prim_indices[i] = static_cast<uint32_t>(indices[i]);
			}

			break;
		}
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
			const uint32_t* indices = reinterpret_cast<const uint32_t*>(
					&index_buffer.data[index_view.byteOffset + index_accessor.byteOffset]);
			for (size_t i = 0; i < index_count; ++i) {
				prim_indices[i] = indices[i];
			}

			break;
		}
		default:
			GL_ASSERT(false, "Unsupported index type");
	}

	return StaticMesh::create(prim_vertices, prim_indices);
}

AssetHandle _load_material(int p_material_index, GLTFLoadContext& p_ctx) {
	if (p_material_index < 0 || p_material_index >= p_ctx.model->materials.size()) {
		return s_default_material;
	}

	// Check Cache
	if (auto it = p_ctx.loaded_materials.find(p_material_index);
			it != p_ctx.loaded_materials.end()) {
		return it->second;
	}

	const tinygltf::Material& gltf_material = p_ctx.model->materials[p_material_index];

	std::optional<AssetHandle> handle_opt =
			AssetSystem::create<Material>(DEFINITION_PATH_PBR_STANDARD);

	if (!handle_opt) {
		GL_LOG_ERROR("[GLTFLoader::_load_material] Failed to create material asset for GLTF "
					 "material: {}",
				gltf_material.name);
		return s_default_material;
	}

	const AssetHandle handle = *handle_opt;
	std::shared_ptr<Material> material = AssetSystem::get<Material>(*handle_opt);

	// Set Parameters
	const auto& base_color = gltf_material.pbrMetallicRoughness.baseColorFactor;

	if (const auto it = gltf_material.extensions.find("KHR_materials_pbrSpecularGlossiness");
			it != gltf_material.extensions.end()) {
		const tinygltf::Value& specGloss = it->second;

		const auto& diffuse_factor = specGloss.Has("diffuseFactor")
				? specGloss.Get("diffuseFactor").Get<tinygltf::Value::Array>()
				: tinygltf::Value::Array{ tinygltf::Value(1.0), tinygltf::Value(1.0),
					  tinygltf::Value(1.0), tinygltf::Value(1.0) };

		material->set_param("base_color",
				glm::vec4(float(diffuse_factor[0].GetNumberAsDouble()),
						float(diffuse_factor[1].GetNumberAsDouble()),
						float(diffuse_factor[2].GetNumberAsDouble()),
						float(diffuse_factor[3].GetNumberAsDouble())));

		const int diffuse_texture_index = _get_extension_texture_index(specGloss, "diffuseTexture");
		AssetHandle diffuse_texture = (diffuse_texture_index >= 0)
				? _load_texture(diffuse_texture_index, p_ctx)
				: s_default_texture;
		material->set_param("u_diffuse_texture", diffuse_texture);

		const int specular_texture_index =
				_get_extension_texture_index(specGloss, "specularGlossinessTexture");
		AssetHandle specular_texture = (specular_texture_index >= 0)
				? _load_texture(specular_texture_index, p_ctx)
				: s_default_texture;
		material->set_param("u_metallic_roughness_texture", specular_texture);

		material->set_param("metallic", 0.0f);
		material->set_param("roughness", 1.0f);
	} else {
		const auto& base_color = gltf_material.pbrMetallicRoughness.baseColorFactor;
		material->set_param("base_color",
				glm::vec4(base_color[0], base_color[1], base_color[2], base_color[3]));

		material->set_param(
				"metallic", static_cast<float>(gltf_material.pbrMetallicRoughness.metallicFactor));
		material->set_param("roughness",
				static_cast<float>(gltf_material.pbrMetallicRoughness.roughnessFactor));

		const int albedo_texture_index = gltf_material.pbrMetallicRoughness.baseColorTexture.index;
		AssetHandle albedo_texture = (albedo_texture_index >= 0)
				? _load_texture(albedo_texture_index, p_ctx)
				: s_default_texture;
		material->set_param("u_diffuse_texture", albedo_texture);

		const int metallic_roughness_texture_index =
				gltf_material.pbrMetallicRoughness.metallicRoughnessTexture.index;
		AssetHandle metallic_roughness_texture = (metallic_roughness_texture_index >= 0)
				? _load_texture(metallic_roughness_texture_index, p_ctx)
				: s_default_texture;
		material->set_param("u_metallic_roughness_texture", metallic_roughness_texture);
	}

	// Common maps
	const int norm_index = gltf_material.normalTexture.index;
	material->set_param("u_normal_texture",
			(norm_index >= 0) ? _load_texture(norm_index, p_ctx) : s_default_texture);

	const int occ_index = gltf_material.occlusionTexture.index;
	material->set_param("u_ambient_occlusion_texture",
			(occ_index >= 0) ? _load_texture(occ_index, p_ctx) : s_default_texture);

	material->upload();

	// Cache and return
	p_ctx.loaded_materials[p_material_index] = handle;
	return handle;
}

AssetHandle _load_texture(int p_texture_index, GLTFLoadContext& p_ctx) {
	size_t hash = 0;
	hash_combine(hash, p_texture_index);
	hash_combine(hash, p_ctx.model_hash);

	if (auto it = p_ctx.loaded_textures.find(hash); it != p_ctx.loaded_textures.end()) {
		return it->second;
	}

	const tinygltf::Texture& gltf_texture = p_ctx.model->textures[p_texture_index];
	const tinygltf::Image& gltf_image = p_ctx.model->images[gltf_texture.source];

	TextureSamplerOptions sampler_options = {};
	if (gltf_texture.sampler >= 0) {
		const tinygltf::Sampler& sampler = p_ctx.model->samplers[gltf_texture.sampler];

		sampler_options.mag_filter = _gltf_to_image_filtering(sampler.magFilter);
		sampler_options.min_filter = _gltf_to_image_filtering(sampler.minFilter);

		sampler_options.wrap_u = _gltf_to_image_wrapping(sampler.wrapS);
		sampler_options.wrap_v = _gltf_to_image_wrapping(sampler.wrapT);
	}

	AssetHandle texture_handle;
	if (gltf_image.uri.empty()) {
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

		texture_handle = AssetSystem::register_asset(
				Texture::create(format, glm::uvec2(gltf_image.width, gltf_image.height),
						gltf_image.image.data(), sampler_options),
				std::format(
						"mem://Texture/GLTF/?id={}&&model={}", p_ctx.model_hash, p_texture_index));
	} else {
		const fs::path texture_path = p_ctx.base_path / gltf_image.uri;

		auto texture = Texture::load_from_file(texture_path, sampler_options);
		if (!texture) {
			GL_LOG_ERROR("[GLTFLoader::_load_texture] Unable to load GLTF texture from path '{}'",
					texture_path.string());
			return INVALID_ASSET_HANDLE;
		}

		texture_handle = AssetSystem::register_asset(texture,
				std::format("mem://Texture/GLTF/?path={}&&model={}", p_ctx.model_hash,
						texture_path.string()));
	}

	p_ctx.loaded_textures[hash] = texture_handle;

	return texture_handle;
}

size_t _hash_gltf_model(const tinygltf::Model& p_model) {
	size_t seed = 0;

	for (const auto& buffer : p_model.buffers) {
		if (!buffer.data.empty()) {
			hash_combine(
					seed, hash64(buffer.data.data(), std::min<size_t>(buffer.data.size(), 1024)));
		}
	}

	for (const auto& mesh : p_model.meshes) {
		for (const auto& primitive : mesh.primitives) {
			for (const auto& attr : primitive.attributes) {
				hash_combine(seed, hash64(attr.first));
				hash_combine(seed, hash64(attr.second));
			}
			hash_combine(seed, hash64(primitive.indices));
		}
	}

	for (const auto& material : p_model.materials) {
		hash_combine(seed, hash64(material.name));
		hash_combine(seed, hash64(material.pbrMetallicRoughness.baseColorFactor));
		hash_combine(seed, hash64(material.pbrMetallicRoughness.metallicFactor));
		hash_combine(seed, hash64(material.pbrMetallicRoughness.roughnessFactor));
	}

	for (const auto& image : p_model.images) {
		hash_combine(seed, hash64(image.name));
		hash_combine(seed, hash64(image.image.size()));
	}

	return seed;
}

} //namespace gl
