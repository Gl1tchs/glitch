/**
 * @file mesh.h
 *
 */

#pragma once

#include "glitch/renderer/frustum.h"
#include "glitch/renderer/material.h"
#include "glitch/renderer/types.h"

namespace gl {

typedef uint64_t MeshHandle;

struct MeshVertex {
	glm::vec3 position;
	float uv_x;
	glm::vec3 normal;
	float uv_y;
};

struct GL_API MeshPrimitive {
	Buffer vertex_buffer;
	Buffer index_buffer;
	BufferDeviceAddress vertex_buffer_address;
	uint32_t index_count;

	Ref<MaterialInstance> material;

	AABB aabb;

	~MeshPrimitive();

	static Ref<MeshPrimitive> create(const std::span<MeshVertex>& p_vertices,
			const std::span<uint32_t>& p_indices);
};

struct Mesh {
	std::vector<Ref<MeshPrimitive>> primitives;
};

} //namespace gl