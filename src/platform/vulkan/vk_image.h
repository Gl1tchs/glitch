#pragma once

#include "core/templates/bit_field.h"

#include "renderer/types.h"

#include "platform/vulkan/vk_common.h"

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
		BitField<ImageUsageBits> p_usage = IMAGE_USAGE_SAMPLED_BIT,
		bool p_mipmapped = false);

void image_free(Context p_context, Image p_image);

Vec3u image_get_size(Image p_image);

}; //namespace vk
