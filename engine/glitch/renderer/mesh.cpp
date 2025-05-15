#include "glitch/renderer/mesh.h"

#include "glitch/renderer/render_backend.h"
#include "glitch/renderer/renderer.h"

MeshPrimitive::~MeshPrimitive() {
	Ref<RenderBackend> backend = Renderer::get_backend();

	backend->buffer_free(vertex_buffer);
	backend->buffer_free(index_buffer);
}

Ref<MeshPrimitive> MeshPrimitive::create(
		std::vector<MeshVertex> p_vertices, std::vector<uint32_t> p_indices) {
	if (p_vertices.empty() || p_indices.empty()) {
		return nullptr;
	}

	Ref<RenderBackend> backend = Renderer::get_backend();
	Ref<MeshPrimitive> primitive = create_ref<MeshPrimitive>();

	// TODO! use staging buffer

	// Upload vertex buffer
	primitive->vertex_buffer =
			backend->buffer_create(p_vertices.size() * sizeof(MeshVertex),
					BUFFER_USAGE_STORAGE_BUFFER_BIT |
							BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
							BUFFER_USAGE_TRANSFER_DST_BIT,
					MEMORY_ALLOCATION_TYPE_CPU);

	void* mapped_vertex_data = backend->buffer_map(primitive->vertex_buffer);
	std::memcpy(mapped_vertex_data, p_vertices.data(),
			p_vertices.size() * sizeof(MeshVertex));
	backend->buffer_unmap(primitive->vertex_buffer);

	// Upload index buffer
	primitive->index_buffer = backend->buffer_create(
			p_indices.size() * sizeof(uint32_t),
			BUFFER_USAGE_INDEX_BUFFER_BIT | BUFFER_USAGE_TRANSFER_DST_BIT,
			MEMORY_ALLOCATION_TYPE_CPU);

	void* mapped_index_data = backend->buffer_map(primitive->index_buffer);
	std::memcpy(mapped_index_data, p_indices.data(),
			p_indices.size() * sizeof(uint32_t));
	backend->buffer_unmap(primitive->index_buffer);

	primitive->vertex_buffer_address =
			backend->buffer_get_device_address(primitive->vertex_buffer);
	primitive->index_count = p_indices.size();

	return primitive;
}
