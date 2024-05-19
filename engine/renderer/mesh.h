#pragma once

#include "renderer/vertex.h"

struct Mesh {
	std::span<Vertex> vertices;
	std::span<uint32_t> indices;

	virtual ~Mesh();

	static Ref<Mesh> create(const std::span<Vertex> vertices,
			const std::span<uint32_t> indices);
};

struct ModelMesh {
	uint32_t start_index;
	uint32_t count;
};

struct Model {
	Ref<Mesh> mesh;
	std::vector<ModelMesh> surfaces;
};
