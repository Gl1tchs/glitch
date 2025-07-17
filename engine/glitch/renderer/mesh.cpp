#include "glitch/renderer/mesh.h"

#include "glitch/renderer/render_backend.h"
#include "glitch/renderer/renderer.h"

namespace gl {

static AABB _get_aabb_from_vertices(const std::span<MeshVertex>& p_vertices) {
	glm::vec3 min = glm::vec3(std::numeric_limits<float>::max());
	glm::vec3 max = glm::vec3(std::numeric_limits<float>::lowest());

	for (const auto& v : p_vertices) {
		min = glm::min(min, v.position);
		max = glm::max(max, v.position);
	}

	return { min, max };
}

MeshPrimitive::~MeshPrimitive() {
	Ref<RenderBackend> backend = Renderer::get_backend();

	backend->buffer_free(vertex_buffer);
	backend->buffer_free(index_buffer);
}

Ref<MeshPrimitive> MeshPrimitive::create(
		const std::span<MeshVertex>& p_vertices,
		const std::span<uint32_t>& p_indices) {
	if (p_vertices.empty() || p_indices.empty()) {
		return nullptr;
	}

	Ref<RenderBackend> backend = Renderer::get_backend();
	Ref<MeshPrimitive> primitive = create_ref<MeshPrimitive>();

	const size_t vertex_size = p_vertices.size() * sizeof(MeshVertex);
	const size_t index_size = p_indices.size() * sizeof(uint32_t);

	const size_t data_size = vertex_size + index_size;

	Buffer staging_buffer = backend->buffer_create(data_size,
			BUFFER_USAGE_TRANSFER_SRC_BIT, MemoryAllocationType::CPU);

	uint8_t* mapped_data = backend->buffer_map(staging_buffer);
	{
		// Copy vertex data
		memcpy(mapped_data, p_vertices.data(), vertex_size);

		// Copy index data
		memcpy(mapped_data + vertex_size, p_indices.data(), index_size);
	}
	backend->buffer_unmap(staging_buffer);

	// Create vertex buffer
	primitive->vertex_buffer =
			backend->buffer_create(p_vertices.size() * sizeof(MeshVertex),
					BUFFER_USAGE_STORAGE_BUFFER_BIT |
							BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
							BUFFER_USAGE_TRANSFER_DST_BIT,
					MemoryAllocationType::GPU);

	// Create index buffer
	primitive->index_buffer = backend->buffer_create(
			p_indices.size() * sizeof(uint32_t),
			BUFFER_USAGE_INDEX_BUFFER_BIT | BUFFER_USAGE_TRANSFER_DST_BIT,
			MemoryAllocationType::GPU);

	backend->command_immediate_submit([&](CommandBuffer p_cmd) {
		BufferCopyRegion region;

		// Copy vertex buffer
		region.src_offset = 0;
		region.size = vertex_size;
		region.dst_offset = 0;

		backend->command_copy_buffer(
				p_cmd, staging_buffer, primitive->vertex_buffer, region);

		// Copy index buffer
		region.src_offset = vertex_size;
		region.size = index_size;
		region.dst_offset = 0;

		backend->command_copy_buffer(
				p_cmd, staging_buffer, primitive->index_buffer, region);
	});

	backend->buffer_free(staging_buffer);

	primitive->vertex_buffer_address =
			backend->buffer_get_device_address(primitive->vertex_buffer);
	primitive->index_count = p_indices.size();
	primitive->aabb = _get_aabb_from_vertices(p_vertices);

	return primitive;
}

} //namespace gl