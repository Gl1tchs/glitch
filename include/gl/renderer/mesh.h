#pragma once

#include "gl/renderer/vertex.h"

struct Mesh {
	std::span<Vertex> vertices;
	std::span<uint32_t> indices;

	virtual ~Mesh() = default;

	static Ref<Mesh> create(const std::span<Vertex> vertices,
			const std::span<uint32_t> indices);

	static void destroy(Ref<Mesh> mesh);
};

struct ModelMesh {
	uint32_t start_index;
	uint32_t count;
};

struct Model {
	Ref<Mesh> mesh;
	std::vector<ModelMesh> surfaces;
};
