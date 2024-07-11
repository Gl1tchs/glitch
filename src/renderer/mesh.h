#pragma once

#include "renderer/material.h"
#include "renderer/types.h"

struct Vertex {
	glm::vec3 position;
	float uv_x;
	glm::vec3 normal;
	float uv_y;
};

struct Bounds {
	glm::vec3 origin;
	float sphere_radius;
	glm::vec3 extents;
};

struct Mesh {
	std::string name = "";

	Bounds bounds;

	Buffer vertex_buffer;
	Buffer index_buffer;
	BufferDeviceAddress vertex_buffer_address;

	uint32_t index_count;
	IndexType index_type;

	uint32_t vertex_count;

	Ref<MaterialInstance> material;

	int model_index;

	int diffuse_index;
	int specular_index;

	static Ref<Mesh> create(std::span<Vertex> p_vertices,
			std::span<uint32_t> p_indices, IndexType p_index_type);

	static void destroy(const Mesh* p_mesh);
};
