/**
 * @file mesh.h
 *
 */

#pragma once

#include "glitch/asset/asset.h"
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

	std::shared_ptr<MaterialInstance> material;

	AABB aabb;

	~MeshPrimitive();

	static std::shared_ptr<MeshPrimitive> create(
			const std::span<MeshVertex>& p_vertices, const std::span<uint32_t>& p_indices);
};

struct StaticMesh {
	GL_REFLECT_ASSET("Mesh")

	~StaticMesh();

	std::vector<std::shared_ptr<MeshPrimitive>> primitives;
};

} //namespace gl
