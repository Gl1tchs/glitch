#include "platform/vulkan/vk_mesh.h"

#include "platform/vulkan/vk_renderer.h"

Ref<VulkanMesh> VulkanMesh::create(const VulkanContext& context,
		const std::span<Vertex> vertices, const std::span<uint32_t> indices) {
	Ref<VulkanMesh> mesh = create_ref<VulkanMesh>();
	mesh->index_count = indices.size();

	const uint32_t vertex_buffer_size = vertices.size() * sizeof(Vertex);
	const uint32_t index_buffer_size = indices.size() * sizeof(uint32_t);

	mesh->vertex_buffer =
			VulkanBuffer::create(context.allocator, vertex_buffer_size,
					VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
							VK_BUFFER_USAGE_TRANSFER_DST_BIT |
							VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
					VMA_MEMORY_USAGE_GPU_ONLY);

	// get the address
	VkBufferDeviceAddressInfo address_info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
		.buffer = mesh->vertex_buffer.buffer,
	};
	mesh->vertex_buffer_address =
			vkGetBufferDeviceAddress(context.device, &address_info);

	mesh->index_buffer = VulkanBuffer::create(context.allocator,
			index_buffer_size,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VMA_MEMORY_USAGE_GPU_ONLY);

	VulkanBuffer staging = VulkanBuffer::create(context.allocator,
			vertex_buffer_size + index_buffer_size,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

	void* data = staging.info.pMappedData;

	// copy vertex data
	memcpy(data, vertices.data(), vertex_buffer_size);
	// copy index data
	memcpy((char*)data + vertex_buffer_size, indices.data(), index_buffer_size);

	VulkanRenderer::get_instance()->immediate_submit(
			[&](VulkanCommandBuffer& cmd) {
				VkBufferCopy vertex_copy = {
					.srcOffset = 0,
					.dstOffset = 0,
					.size = vertex_buffer_size,
				};
				cmd.copy_buffer(staging, mesh->vertex_buffer, 1, &vertex_copy);

				VkBufferCopy index_copy = {
					.srcOffset = vertex_buffer_size,
					.dstOffset = 0,
					.size = index_buffer_size,
				};
				cmd.copy_buffer(staging, mesh->index_buffer, 1, &index_copy);
			});

	VulkanBuffer::destroy(context.allocator, staging);

	return mesh;
}

void VulkanMesh::destroy(const VulkanContext& context, const VulkanMesh* mesh) {
	VulkanBuffer::destroy(context.allocator, mesh->vertex_buffer);
	VulkanBuffer::destroy(context.allocator, mesh->index_buffer);
}
