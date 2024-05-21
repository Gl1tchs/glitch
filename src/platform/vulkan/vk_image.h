#pragma once

#include "gl/renderer/image.h"

#include "platform/vulkan/vk_common.h"

[[nodiscard]] VkFormat image_format_to_vk_format(ImageFormat format);

struct VulkanImage : public Image {
	VkImage image;
	VkImageView image_view;
	VmaAllocation allocation;
	VkExtent3D image_extent;
	VkFormat image_format;

	virtual ~VulkanImage() = default;

	static Ref<VulkanImage> create(const VulkanContext& context,
			VkExtent3D size, VkFormat format, VkImageUsageFlags usage,
			bool mipmapped = false);

	static Ref<VulkanImage> create(const VulkanContext& context,
			const void* data, VkExtent3D size, VkFormat format,
			VkImageUsageFlags usage, bool mipmapped = false);

	static void destroy(const VulkanContext& context, VulkanImage* img);
};
