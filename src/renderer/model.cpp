#include "gl/renderer/model.h"

#include "gl/renderer/renderer.h"

#include "platform/vulkan/vk_model.h"
#include "platform/vulkan/vk_renderer.h"

#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>

namespace fastgltf {

template <>
struct ElementTraits<Vec2f>
		: ElementTraitsBase<Vec2f, AccessorType::Vec2, float> {};

template <>
struct ElementTraits<Vec3f>
		: ElementTraitsBase<Vec3f, AccessorType::Vec3, float> {};

template <>
struct ElementTraits<Vec4f>
		: ElementTraitsBase<Vec4f, AccessorType::Vec4, float> {};

} // namespace fastgltf

std::optional<std::vector<Ref<Model>>> Model::load(const fs::path& path) {
	auto data = fastgltf::GltfDataBuffer::FromPath(path);
	if (data.error() != fastgltf::Error::None) {
		return {};
	}

	constexpr auto gltf_options = fastgltf::Options::LoadExternalBuffers;

	fastgltf::Asset gltf;
	fastgltf::Parser parser{};

	auto load =
			parser.loadGltfBinary(data.get(), path.parent_path(), gltf_options);
	if (load) {
		gltf = std::move(load.get());
	} else {
		GL_LOG_ERROR("Failed to load glTF: {} \n",
				fastgltf::to_underlying(load.error()));
		return {};
	}

	std::vector<Ref<Model>> models;

	// use the same vectors for all meshes so that the memory doesnt reallocate
	// as often
	std::vector<uint32_t> indices;
	std::vector<Vertex> vertices;
	for (fastgltf::Mesh& mesh : gltf.meshes) {
		// clear the mesh arrays each mesh, we dont want to merge them by error
		indices.clear();
		vertices.clear();

		std::vector<Mesh> model_meshes;

		for (auto&& p : mesh.primitives) {
			Mesh new_mesh;
			new_mesh.start_index = (uint32_t)indices.size();
			new_mesh.index_count =
					(uint32_t)gltf.accessors[p.indicesAccessor.value()].count;

			const size_t initial_vtx = vertices.size();

			// load indexes
			{
				fastgltf::Accessor& index_accessor =
						gltf.accessors[p.indicesAccessor.value()];
				indices.reserve(indices.size() + index_accessor.count);

				fastgltf::iterateAccessor<uint32_t>(
						gltf, index_accessor, [&](uint32_t idx) {
							indices.push_back(idx + initial_vtx);
						});
			}

			// load vertex positions
			{
				fastgltf::Accessor& pos_accessor =
						gltf.accessors[p.findAttribute("POSITION")->second];
				vertices.resize(vertices.size() + pos_accessor.count);

				fastgltf::iterateAccessorWithIndex<Vec3f>(
						gltf, pos_accessor, [&](Vec3f v, size_t index) {
							Vertex new_vtx;
							new_vtx.position = v;
							new_vtx.normal = { 1, 0, 0 };
							new_vtx.color = Vec4f{ 1.f };
							new_vtx.uv_x = 0;
							new_vtx.uv_y = 0;
							vertices[initial_vtx + index] = new_vtx;
						});
			}

			// load vertex normals
			auto normals = p.findAttribute("NORMAL");
			if (normals != p.attributes.end()) {
				fastgltf::iterateAccessorWithIndex<Vec3f>(gltf,
						gltf.accessors[(*normals).second],
						[&](Vec3f v, size_t index) {
							vertices[initial_vtx + index].normal = v;
						});
			}

			// load UVs
			auto uv = p.findAttribute("TEXCOORD_0");
			if (uv != p.attributes.end()) {
				fastgltf::iterateAccessorWithIndex<Vec2f>(gltf,
						gltf.accessors[(*uv).second],
						[&](Vec2f v, size_t index) {
							vertices[initial_vtx + index].uv_x = v.x;
							vertices[initial_vtx + index].uv_y = v.y;
						});
			}

			// load vertex colors
			auto colors = p.findAttribute("COLOR_0");
			if (colors != p.attributes.end()) {
				fastgltf::iterateAccessorWithIndex<Vec4f>(gltf,
						gltf.accessors[(*colors).second],
						[&](Vec4f v, size_t index) {
							vertices[initial_vtx + index].color = v;
						});
			}

			model_meshes.push_back(new_mesh);
		}

		// display the vertex normals
		constexpr bool OVERRIDE_COLORS = true;
		if (OVERRIDE_COLORS) {
			for (Vertex& vtx : vertices) {
				vtx.color =
						Vec4f(vtx.normal.x, vtx.normal.y, vtx.normal.z, 1.0f);
			}
		}

		Ref<Model> new_model = Model::create(vertices, indices);
		new_model->name = mesh.name;
		new_model->meshes = model_meshes;

		models.emplace_back(Ref<Model>(std::move(new_model)));
	}

	return models;
}

Ref<Model> Model::create(
		const std::span<Vertex> vertices, const std::span<uint32_t> indices) {
	switch (Renderer::get_backend()) {
		case RenderBackend::Vulkan: {
			Ref<VulkanModel> vk_model = VulkanModel::create(
					VulkanRenderer::get_context(), vertices, indices);

			Mesh mesh;
			mesh.start_index = 0;
			mesh.index_count = indices.size();

			vk_model->meshes.push_back(mesh);

			return vk_model;
		}

		default: {
			return nullptr;
		}
	}
}

void Model::destroy(Model* model) {
	if (model->destroyed) {
		return;
	}

	switch (Renderer::get_backend()) {
		case RenderBackend::Vulkan: {
			VulkanModel* vk_model = reinterpret_cast<VulkanModel*>(model);
			VulkanModel::destroy(VulkanRenderer::get_context(), vk_model);

			model->destroyed = true;

			break;
		}
		default: {
			break;
		}
	}
}
