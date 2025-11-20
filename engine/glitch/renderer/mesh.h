/**
 * @file mesh.h
 *
 */

#pragma once

#include "glitch/asset/asset.h"
#include "glitch/renderer/frustum.h"
#include "glitch/renderer/types.h"

namespace gl {

typedef uint64_t MeshHandle;

struct MeshVertex {
	glm::vec3 position;
	float uv_x;
	glm::vec3 normal;
	float uv_y;
};

/**
 * Asset type of Mesh defining a static mesh primitive.
 *
 */
struct GL_API StaticMesh {
	GL_REFLECT_ASSET("Mesh")

	Buffer vertex_buffer;
	Buffer index_buffer;
	BufferDeviceAddress vertex_buffer_address;
	uint32_t index_count;

	AABB aabb;

	~StaticMesh();

	static std::shared_ptr<StaticMesh> create(
			const std::span<MeshVertex>& p_vertices, const std::span<uint32_t>& p_indices);
};

} //namespace gl