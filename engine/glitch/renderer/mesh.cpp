#include "glitch/renderer/mesh.h"

#include "glitch/renderer/renderer.h"

#include <cstring>

namespace gl {

static AABB _get_aabb_from_vertices(const std::span<MeshVertex>& vertices) {
	Vec3f min = Vec3f(std::numeric_limits<float>::max());
	Vec3f max = Vec3f(std::numeric_limits<float>::lowest());

	for (const auto& v : vertices) {
		min = math::min(min, v.position);
		max = math::max(max, v.position);
	}

	return { min, max };
}

StaticMesh::~StaticMesh() {
	auto device = Renderer::get_device();

	device->device_wait();

	device->buffer_free(vertex_buffer);
	device->buffer_free(index_buffer);
}

std::shared_ptr<StaticMesh> StaticMesh::create(
		const std::span<MeshVertex>& vertices, const std::span<uint32_t>& indices) {
	if (vertices.empty() || indices.empty()) {
		return nullptr;
	}

	auto device = Renderer::get_device();
	std::shared_ptr<StaticMesh> smesh = std::make_shared<StaticMesh>();

	const size_t vertex_size = vertices.size() * sizeof(MeshVertex);
	const size_t index_size = indices.size() * sizeof(uint32_t);

	const size_t data_size = vertex_size + index_size;

	Buffer staging_buffer = device->buffer_create(data_size, BUFFER_USAGE_TRANSFER_SRC_BIT,
										  MemoryAllocationType::CPU)
									.value();

	uint8_t* mapped_data = device->buffer_map(staging_buffer).value();
	{
		// Copy vertex data
		memcpy(mapped_data, vertices.data(), vertex_size);

		// Copy index data
		memcpy(mapped_data + vertex_size, indices.data(), index_size);
	}
	device->buffer_unmap(staging_buffer);

	// Create vertex buffer
	smesh->vertex_buffer =
			device->buffer_create(vertices.size() * sizeof(MeshVertex),
						  BUFFER_USAGE_STORAGE_BUFFER_BIT | BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
								  BUFFER_USAGE_TRANSFER_DST_BIT,
						  MemoryAllocationType::GPU)
					.value();

	// Create index buffer
	smesh->index_buffer =
			device->buffer_create(indices.size() * sizeof(uint32_t),
						  BUFFER_USAGE_INDEX_BUFFER_BIT | BUFFER_USAGE_TRANSFER_DST_BIT,
						  MemoryAllocationType::GPU)
					.value();

	device->command_immediate_submit([&](CommandBuffer cmd) {
		BufferCopyRegion region;

		// Copy vertex buffer
		region.src_offset = 0;
		region.size = vertex_size;
		region.dst_offset = 0;

		device->command_copy_buffer(cmd, staging_buffer, smesh->vertex_buffer, region);

		// Copy index buffer
		region.src_offset = vertex_size;
		region.size = index_size;
		region.dst_offset = 0;

		device->command_copy_buffer(cmd, staging_buffer, smesh->index_buffer, region);
	});

	device->buffer_free(staging_buffer);

	smesh->vertex_buffer_address = device->buffer_get_device_address(smesh->vertex_buffer).value();
	smesh->index_count = indices.size();
	smesh->aabb = _get_aabb_from_vertices(vertices);

	return smesh;
}

} //namespace gl