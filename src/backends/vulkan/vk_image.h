#pragma once

#include "backends/vulkan/vk_common.h"

[[nodiscard]] VkFormat image_format_to_vk_format(ImageFormat format);

struct VulkanImageCreateInfo {
	VkFormat format;
	VkExtent3D size;
	void* data = nullptr;
	VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT;
	bool mipmapped = false;
};

struct VulkanImage : public Image {
	VkImage image;
	VkImageView image_view;
	VmaAllocation allocation;
	VkExtent3D image_extent;
	VkFormat image_format;

	virtual ~VulkanImage() = default;

	static Ref<VulkanImage> create(
			const VulkanContext& context, const VulkanImageCreateInfo* info);

	static void destroy(const VulkanContext& context, VulkanImage* img);
};
