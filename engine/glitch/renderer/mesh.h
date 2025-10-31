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

class GL_API MeshSystem {
public:
	static void free_all();

	static MeshHandle register_mesh(Ref<Mesh> p_mesh);
	
	/**
	 * @returns Ref<Mesh> if found `nullptr` otherwise 
	 */
	static Ref<Mesh> get_mesh(MeshHandle p_handle);
	
	/**
	 * @param p_handle handle to find and delete
	 * @returns `true` if deletion successfull `false` if mesh do not exists 
	 */
	static bool free_mesh(MeshHandle p_handle);
};

} //namespace gl