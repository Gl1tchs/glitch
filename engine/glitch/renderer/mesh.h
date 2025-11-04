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

	std::shared_ptr<MaterialInstance> material;

	AABB aabb;

	~MeshPrimitive();

	static std::shared_ptr<MeshPrimitive> create(
			const std::span<MeshVertex>& p_vertices, const std::span<uint32_t>& p_indices);
};

struct Mesh {
	std::vector<std::shared_ptr<MeshPrimitive>> primitives;
};

class GL_API MeshSystem {
public:
	static void free_all();

	static MeshHandle register_mesh(std::shared_ptr<Mesh> p_mesh);

	/**
	 * @returns std::shared_ptr<Mesh> if found `nullptr` otherwise
	 */
	static std::shared_ptr<Mesh> get_mesh(MeshHandle p_handle);

	/**
	 * @param p_handle handle to find and delete
	 * @returns `true` if deletion successfull `false` if mesh do not exists
	 */
	static bool free_mesh(MeshHandle p_handle);
};

} //namespace gl