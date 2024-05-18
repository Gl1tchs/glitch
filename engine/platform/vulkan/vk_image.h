#pragma once

#include "platform/vulkan/vk_common.h"

struct VulkanImage {
	VkImage image;
	VkImageView image_view;
	VmaAllocation allocation;
	VkExtent3D image_extent;
	VkFormat image_format;

	static VulkanImage create(const VulkanContext& context, VkExtent3D size,
			VkFormat format, VkImageUsageFlags usage, bool mipmapped = false);

	static VulkanImage create(const VulkanContext& context, const void* data,
			VkExtent3D size, VkFormat format, VkImageUsageFlags usage,
			bool mipmapped = false);

	static void destroy(const VulkanContext& context, const VulkanImage& img);

	static void transition_image(VkCommandBuffer cmd, VkImage image,
			VkImageLayout current_layout, VkImageLayout new_layout);

	static void copy_image_to_image(VkCommandBuffer cmd, VkImage source,
			VkImage destination, VkExtent2D src_size, VkExtent2D dst_size);
};
