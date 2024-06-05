#pragma once

#include "renderer/render_backend.h"
#include "renderer/types.h"

#include "backends/vulkan/vk_common.h"

struct VulkanImage {
	VkImage vk_image;
	VkImageView vk_image_view;
	VmaAllocation allocation;
	VkExtent3D image_extent;
	VkFormat image_format;
};

namespace vk {

Image image_create(Context p_context, DataFormat p_format, Vec2u p_size,
		const void* p_data = nullptr,
		ImageUsage p_usage = IMAGE_USAGE_SAMPLED_BIT, bool p_mipmapped = false);

void image_free(Context p_context, Image p_image);

}; //namespace vk
