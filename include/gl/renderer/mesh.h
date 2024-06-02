#pragma once

#include "gl/renderer/material.h"
#include "gl/renderer/node.h"
#include "gl/renderer/vertex.h"

struct Mesh : public Node {
	GL_IMPL_NODE(NodeType::GEOMETRY)

	std::string name;

	uint32_t index_count;
	uint32_t vertex_count;

	Ref<MaterialInstance> material;

	int color_index;
	int roughness_index;

	virtual ~Mesh() = default;

	/**
	 * @brief get scene graph representation of a model
	 */
	static Ref<Node> load(
			const fs::path& path, Ref<MetallicRoughnessMaterial> material);

	static Ref<Mesh> create(const std::span<Vertex> vertices,
			const std::span<uint32_t> indices);

	static void destroy(Mesh* model);

private:
	bool destroyed;
};
