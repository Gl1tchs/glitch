#pragma once

#include "gl/renderer/model.h"

#include "platform/vulkan/vk_buffer.h"

struct VulkanModel : public Model {
	VulkanBuffer vertex_buffer;
	VulkanBuffer index_buffer;
	VkDeviceAddress vertex_buffer_address;

	virtual ~VulkanModel() = default;

	static Ref<VulkanModel> create(const VulkanContext& context,
			const std::span<Vertex> vertices,
			const std::span<uint32_t> indices);

	static void destroy(const VulkanContext& context, const VulkanModel* mesh);
};
