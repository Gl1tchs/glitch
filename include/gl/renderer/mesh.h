#pragma once

#include "gl/renderer/vertex.h"

struct Mesh {
	uint32_t index_count;

	virtual ~Mesh() = default;

	static Ref<Mesh> create(const std::span<Vertex> vertices,
			const std::span<uint32_t> indices);

	static void destroy(Ref<Mesh> mesh);
};
