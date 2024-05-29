#include "platform/vulkan/vk_model.h"

#include "platform/vulkan/vk_renderer.h"

Ref<VulkanModel> VulkanModel::create(const VulkanContext& context,
		const std::span<Vertex> vertices, const std::span<uint32_t> indices) {
	Ref<VulkanModel> model = create_ref<VulkanModel>();

	const uint32_t vertex_buffer_size = vertices.size() * sizeof(Vertex);
	const uint32_t index_buffer_size = indices.size() * sizeof(uint32_t);

	model->vertex_buffer =
			VulkanBuffer::create(context.allocator, vertex_buffer_size,
					VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
							VK_BUFFER_USAGE_TRANSFER_DST_BIT |
							VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
					VMA_MEMORY_USAGE_GPU_ONLY);

	// get the address
	VkBufferDeviceAddressInfo address_info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
		.buffer = model->vertex_buffer.buffer,
	};
	model->vertex_buffer_address =
			vkGetBufferDeviceAddress(context.device, &address_info);

	model->index_buffer = VulkanBuffer::create(context.allocator,
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
				cmd.copy_buffer(staging, model->vertex_buffer, 1, &vertex_copy);

				VkBufferCopy index_copy = {
					.srcOffset = vertex_buffer_size,
					.dstOffset = 0,
					.size = index_buffer_size,
				};
				cmd.copy_buffer(staging, model->index_buffer, 1, &index_copy);
			});

	VulkanBuffer::destroy(context.allocator, staging);

	return model;
}

void VulkanModel::destroy(
		const VulkanContext& context, const VulkanModel* mesh) {
	VulkanBuffer::destroy(context.allocator, mesh->vertex_buffer);
	VulkanBuffer::destroy(context.allocator, mesh->index_buffer);
}
