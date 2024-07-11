#include "renderer/mesh.h"

#include "renderer/render_backend.h"
#include "renderer/renderer.h"
#include "renderer/types.h"

static Bounds _calculate_mesh_bounds(const std::span<Vertex>& p_vertices) {
	glm::vec3 min_pos = p_vertices.front().position;
	glm::vec3 max_pos = p_vertices.front().position;

	for (size_t i = 0; i < p_vertices.size(); i++) {
		min_pos = glm::min(min_pos, p_vertices[i].position);
		max_pos = glm::max(max_pos, p_vertices[i].position);
	}

	Bounds bounds;
	bounds.origin = (min_pos + max_pos) / 2.0f;
	bounds.extents = (max_pos - min_pos) / 2.0f;
	bounds.sphere_radius = glm::length(bounds.extents);

	return bounds;
}

Ref<Mesh> Mesh::create(std::span<Vertex> p_vertices,
		std::span<uint32_t> p_indices, IndexType p_index_type) {
	Ref<RenderBackend> backend = Renderer::get_backend();

	const uint32_t vertices_size = p_vertices.size() * sizeof(Vertex);
	const uint32_t indices_size = p_indices.size() *
			(p_index_type == INDEX_TYPE_UINT16 ? sizeof(uint16_t)
											   : sizeof(uint32_t));

	Ref<Mesh> mesh = create_ref<Mesh>();
	mesh->bounds = _calculate_mesh_bounds(p_vertices);
	mesh->index_count = p_indices.size();
	mesh->index_type = p_index_type;
	mesh->vertex_count = p_vertices.size();

	mesh->vertex_buffer = backend->buffer_create(vertices_size,
			BUFFER_USAGE_STORAGE_BUFFER_BIT | BUFFER_USAGE_TRANSFER_DST_BIT |
					BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			MEMORY_ALLOCATION_TYPE_GPU);

	// get the address
	mesh->vertex_buffer_address =
			backend->buffer_get_device_address(mesh->vertex_buffer);

	mesh->index_buffer = backend->buffer_create(indices_size,
			BUFFER_USAGE_INDEX_BUFFER_BIT | BUFFER_USAGE_TRANSFER_DST_BIT,
			MEMORY_ALLOCATION_TYPE_GPU);

	Buffer staging_buffer = backend->buffer_create(vertices_size + indices_size,
			BUFFER_USAGE_TRANSFER_SRC_BIT, MEMORY_ALLOCATION_TYPE_CPU);

	void* data = backend->buffer_map(staging_buffer);
	{
		// copy vertex data
		memcpy(data, p_vertices.data(), vertices_size);
		// copy index data
		memcpy((uint8_t*)data + vertices_size, p_indices.data(), indices_size);
	}
	backend->buffer_unmap(staging_buffer);

	backend->command_immediate_submit([&](CommandBuffer p_cmd) {
		BufferCopyRegion vertex_copy = {};
		vertex_copy.src_offset = 0;
		vertex_copy.dst_offset = 0;
		vertex_copy.size = vertices_size;

		backend->command_copy_buffer(
				p_cmd, staging_buffer, mesh->vertex_buffer, vertex_copy);

		BufferCopyRegion index_copy = {};
		index_copy.src_offset = vertices_size;
		index_copy.dst_offset = 0;
		index_copy.size = indices_size;

		backend->command_copy_buffer(
				p_cmd, staging_buffer, mesh->index_buffer, index_copy);
	});

	backend->buffer_free(staging_buffer);

	return mesh;
}

void Mesh::destroy(const Mesh* p_mesh) {
	Ref<RenderBackend> backend = Renderer::get_backend();

	backend->buffer_free(p_mesh->vertex_buffer);
	backend->buffer_free(p_mesh->index_buffer);
}
