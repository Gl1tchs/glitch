/**
 * @file mesh.h
 *
 */

#pragma once

#include "glitch/renderer/material.h"
#include "glitch/renderer/types.h"

typedef uint64_t MeshHandle;

struct MeshVertex {
	glm::vec3 position;
	float uv_x;
	glm::vec3 normal;
	float uv_y;
};

struct GL_API MeshPrimitive {
	std::string name = "";

	Buffer vertex_buffer;
	Buffer index_buffer;
	BufferDeviceAddress vertex_buffer_address;
	uint32_t index_count;

	Ref<MaterialInstance> material;

	~MeshPrimitive();

	static Ref<MeshPrimitive> create(std::vector<MeshVertex> p_vertices,
			std::vector<uint32_t> p_indices);
};

struct Mesh {
	std::string name = "";
	std::vector<Ref<MeshPrimitive>> primitives;
};