#include "platform/vulkan/vk_image.h"

#include "platform/vulkan/vk_buffer.h"
#include "platform/vulkan/vk_init.h"
#include "platform/vulkan/vk_renderer.h"

#include <vulkan/vulkan_core.h>

Ref<VulkanImage> vk_image_create(
		const VulkanContext& context, const VulkanImageCreateInfo* info) {
	Ref<VulkanImage> new_image = create_ref<VulkanImage>();
	new_image->image_extent = info->size;
	new_image->image_format = info->format;

	VkImageCreateInfo img_info =
			vkinit::image_create_info(info->format, info->usage, info->size);
	if (info->mipmapped) {
		img_info.mipLevels =
				static_cast<uint32_t>(std::floor(std::log2(
						std::max(info->size.width, info->size.height)))) +
				1;
	}

	// always allocate images on dedicated GPU memory
	VmaAllocationCreateInfo alloc_info = {
		.usage = VMA_MEMORY_USAGE_GPU_ONLY,
		.requiredFlags =
				VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
	};

	// allocate and create the image
	VK_CHECK(vmaCreateImage(context.allocator, &img_info, &alloc_info,
			&new_image->image, &new_image->allocation, nullptr));

	// if the format is a depth format, we will need to have it use the correct
	// aspect flag
	VkImageAspectFlags aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT;
	if (info->format == VK_FORMAT_D32_SFLOAT) {
		aspect_flags = VK_IMAGE_ASPECT_DEPTH_BIT;
	}

	// build image view for the image
	VkImageViewCreateInfo view_info = vkinit::imageview_create_info(
			info->format, new_image->image, aspect_flags);
	view_info.subresourceRange.levelCount = img_info.mipLevels;

	VK_CHECK(vkCreateImageView(
			context.device, &view_info, nullptr, &new_image->image_view));

	return new_image;
}

Ref<VulkanImage> VulkanImage::create(
		const VulkanContext& context, const VulkanImageCreateInfo* info) {
	if (!info->data) {
		return vk_image_create(context, info);
	} else {
		const size_t data_size =
				info->size.depth * info->size.width * info->size.height * 4;
		VulkanBuffer staging_buffer = VulkanBuffer::create(context.allocator,
				data_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VMA_MEMORY_USAGE_CPU_TO_GPU);

		memcpy(staging_buffer.info.pMappedData, info->data, data_size);

		VkImageUsageFlags image_usage = info->usage;
		image_usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		image_usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

		VulkanImageCreateInfo new_info = *info;
		new_info.usage = image_usage;

		Ref<VulkanImage> new_image = vk_image_create(context, &new_info);

		VulkanRenderer::get_instance()->immediate_submit(
				[&](VulkanCommandBuffer& cmd) {
					cmd.transition_image(new_image, VK_IMAGE_LAYOUT_UNDEFINED,
							VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

					VkBufferImageCopy copy_region = {
                        .bufferOffset = 0,
                        .bufferRowLength = 0,
                        .bufferImageHeight = 0,
                        .imageSubresource = {
                            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                            .mipLevel = 0,
                            .baseArrayLayer = 0,
                            .layerCount = 1,
                        },
                        .imageExtent =info->size,
                    };

					// copy the buffer into the image
					cmd.copy_buffer_to_image(staging_buffer, new_image,
							VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
							&copy_region);

					cmd.transition_image(new_image,
							VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
							VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
				});

		VulkanBuffer::destroy(context.allocator, staging_buffer);

		return new_image;
	}
}

void VulkanImage::destroy(const VulkanContext& context, VulkanImage* img) {
	vkDestroyImageView(context.device, img->image_view, nullptr);
	vmaDestroyImage(context.allocator, img->image, img->allocation);
}

[[nodiscard]] VkFormat image_format_to_vk_format(ImageFormat format) {
	switch (format) {
		case ImageFormat::R8_UNORM: {
			return VK_FORMAT_R8_UNORM;
		}
		case ImageFormat::R8G8_UNORM: {
			return VK_FORMAT_R8G8_UNORM;
		}
		case ImageFormat::R8G8B8_UNORM: {
			return VK_FORMAT_R8G8B8_UNORM;
		}
		case ImageFormat::R8G8B8A8_UNORM: {
			return VK_FORMAT_R8G8B8A8_UNORM;
		}
		case ImageFormat::R16_UNORM: {
			return VK_FORMAT_R16_UNORM;
		}
		case ImageFormat::R16G16_UNORM: {
			return VK_FORMAT_R16G16_UNORM;
		}
		case ImageFormat::R16G16B16_UNORM: {
			return VK_FORMAT_R16G16B16_UNORM;
		}
		case ImageFormat::R16G16B16A16_UNORM: {
			return VK_FORMAT_R16G16B16A16_UNORM;
		}
		case ImageFormat::R16G16B16A16_SFLOAT: {
			return VK_FORMAT_R16G16B16A16_SFLOAT;
		}
		case ImageFormat::R32_SFLOAT: {
			return VK_FORMAT_R32_SFLOAT;
		}
		case ImageFormat::R32G32_SFLOAT: {
			return VK_FORMAT_R32G32_SFLOAT;
		}
		case ImageFormat::R32G32B32_SFLOAT: {
			return VK_FORMAT_R32G32B32_SFLOAT;
		}
		case ImageFormat::R32G32B32A32_SFLOAT: {
			return VK_FORMAT_R32G32B32A32_SFLOAT;
		}
		case ImageFormat::B8G8R8_UNORM: {
			return VK_FORMAT_B8G8R8_UNORM;
		}
		case ImageFormat::B8G8R8A8_UNORM: {
			return VK_FORMAT_B8G8R8A8_UNORM;
		}
		case ImageFormat::D16_UNORM: {
			return VK_FORMAT_D16_UNORM;
		}
		case ImageFormat::D24_UNORM_S8_UINT: {
			return VK_FORMAT_D24_UNORM_S8_UINT;
		}
		case ImageFormat::D32_SFLOAT: {
			return VK_FORMAT_D32_SFLOAT;
		}
		case ImageFormat::D32_SFLOAT_S8_UINT: {
			return VK_FORMAT_D32_SFLOAT_S8_UINT;
		}
		default: {
			return VK_FORMAT_R8G8B8A8_UNORM;
		}
	}
}
