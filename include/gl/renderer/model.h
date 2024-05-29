#pragma once

#include "gl/renderer/material.h"
#include "gl/renderer/node.h"
#include "gl/renderer/vertex.h"

struct Mesh {
	uint32_t start_index;
	uint32_t index_count;
};

struct Model : public Node {
	GL_IMPL_NODE(NodeType::GEOMETRY)

	std::string name;
	std::vector<Mesh> meshes;
	Ref<MaterialInstance> material;

	virtual ~Model() = default;

	static std::optional<std::vector<Ref<Model>>> load(const fs::path& path);

	static Ref<Model> create(const std::span<Vertex> vertices,
			const std::span<uint32_t> indices);

	static void destroy(Model* model);

private:
	bool destroyed;
};
