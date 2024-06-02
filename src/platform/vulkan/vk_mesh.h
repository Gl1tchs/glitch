#pragma once

#include "gl/renderer/mesh.h"

#include "platform/vulkan/vk_buffer.h"

struct VulkanMesh : public Mesh {
	VulkanBuffer vertex_buffer;
	VulkanBuffer index_buffer;
	VkDeviceAddress vertex_buffer_address;

	virtual ~VulkanMesh() = default;

	static Ref<VulkanMesh> create(const VulkanContext& context,
			const std::span<Vertex> vertices,
			const std::span<uint32_t> indices);

	static void destroy(const VulkanContext& context, const VulkanMesh* mesh);
};
