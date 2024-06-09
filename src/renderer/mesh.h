#pragma once

#include "renderer/material.h"
#include "renderer/node.h"
#include "renderer/types.h"

struct Vertex {
	Vec3f position;
	float uv_x;
	Vec3f normal;
	float uv_y;
};

struct Mesh : public Node {
	GL_IMPL_NODE(NodeType::GEOMETRY)

	Buffer vertex_buffer;
	Buffer index_buffer;
	uint64_t vertex_buffer_address;

	uint32_t index_count;
	uint32_t vertex_count;

	IndexType index_type;

	Ref<MaterialInstance> material;

	int color_index;
	int roughness_index;
	int normal_index;

	virtual ~Mesh() = default;

	/**
	 * @brief get scene graph representation of a model
	 */
	static Ref<Node> load(const fs::path& p_path, Ref<Material> p_material);

	static Ref<Mesh> create(
			std::span<Vertex> p_vertices, std::span<uint32_t> p_indices);

	static void destroy(const Mesh* p_mesh);

private:
	bool destroyed;
};
