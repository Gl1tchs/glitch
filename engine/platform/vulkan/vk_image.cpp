#include "platform/vulkan/vk_image.h"

#include "platform/vulkan/vk_buffer.h"
#include "platform/vulkan/vk_init.h"
#include "platform/vulkan/vk_renderer.h"

VulkanImage VulkanImage::create(VkDevice device, VmaAllocator allocator,
		VkExtent3D size, VkFormat format, VkImageUsageFlags usage,
		bool mipmapped) {
	VulkanImage new_image = {
		.image_extent = size,
		.image_format = format,
	};

	VkImageCreateInfo img_info = vkinit::image_create_info(format, usage, size);
	if (mipmapped) {
		img_info.mipLevels = static_cast<uint32_t>(std::floor(std::log2(
									 std::max(size.width, size.height)))) +
				1;
	}

	// always allocate images on dedicated GPU memory
	VmaAllocationCreateInfo alloc_info = {
		.usage = VMA_MEMORY_USAGE_GPU_ONLY,
		.requiredFlags =
				VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
	};

	// allocate and create the image
	VK_CHECK(vmaCreateImage(allocator, &img_info, &alloc_info, &new_image.image,
			&new_image.allocation, nullptr));

	// if the format is a depth format, we will need to have it use the correct
	// aspect flag
	VkImageAspectFlags aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT;
	if (format == VK_FORMAT_D32_SFLOAT) {
		aspect_flags = VK_IMAGE_ASPECT_DEPTH_BIT;
	}

	// build image view for the image
	VkImageViewCreateInfo view_info = vkinit::imageview_create_info(
			format, new_image.image, aspect_flags);
	view_info.subresourceRange.levelCount = img_info.mipLevels;

	VK_CHECK(vkCreateImageView(
			device, &view_info, nullptr, &new_image.image_view));

	return new_image;
}

VulkanImage VulkanImage::create(VkDevice device, VmaAllocator allocator,
		void* data, VkExtent3D size, VkFormat format, VkImageUsageFlags usage,
		bool mipmapped) {
	size_t data_size = size.depth * size.width * size.height * 4;
	VulkanBuffer staging_buffer = VulkanBuffer::create(allocator, data_size,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

	memcpy(staging_buffer.info.pMappedData, data, data_size);

	VulkanImage new_image = VulkanImage::create(device, allocator, size, format,
			usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
					VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
			mipmapped);

	VulkanRenderer::get_instance()->immediate_submit([&](VkCommandBuffer cmd) {
		VulkanImage::transition_image(cmd, new_image.image,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		VkBufferImageCopy copy_region = {
            .bufferOffset =0,
            .bufferRowLength = 0,
            .bufferImageHeight =0,
            .imageSubresource = {
                .aspectMask =VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0,
                .baseArrayLayer =0,
                .layerCount = 1,
            },
            .imageExtent =size,
        };

		// copy the buffer into the image
		vkCmdCopyBufferToImage(cmd, staging_buffer.buffer, new_image.image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);

		VulkanImage::transition_image(cmd, new_image.image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	});

	VulkanBuffer::destroy(allocator, staging_buffer);

	return new_image;
}

void VulkanImage::destroy(
		VkDevice device, VmaAllocator allocator, const VulkanImage& img) {
	vkDestroyImageView(device, img.image_view, nullptr);
	vmaDestroyImage(allocator, img.image, img.allocation);
}

void VulkanImage::transition_image(VkCommandBuffer cmd, VkImage image,
		VkImageLayout current_layout, VkImageLayout new_layout) {
	VkImageMemoryBarrier2 image_barrier{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2
	};
	image_barrier.pNext = nullptr;

	image_barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
	image_barrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
	image_barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
	image_barrier.dstAccessMask =
			VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;

	image_barrier.oldLayout = current_layout;
	image_barrier.newLayout = new_layout;

	VkImageAspectFlags aspect_mask =
			(new_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
			? VK_IMAGE_ASPECT_DEPTH_BIT
			: VK_IMAGE_ASPECT_COLOR_BIT;
	image_barrier.subresourceRange =
			vkinit::image_subresource_range(aspect_mask);
	image_barrier.image = image;

	VkDependencyInfo dep_info{};
	dep_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	dep_info.pNext = nullptr;

	dep_info.imageMemoryBarrierCount = 1;
	dep_info.pImageMemoryBarriers = &image_barrier;

	vkCmdPipelineBarrier2(cmd, &dep_info);
}

void VulkanImage::copy_image_to_image(VkCommandBuffer cmd, VkImage source,
		VkImage destination, VkExtent2D src_size, VkExtent2D dst_size) {
	VkImageBlit2 blit_region{ .sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
		.pNext = nullptr };

	blit_region.srcOffsets[1].x = src_size.width;
	blit_region.srcOffsets[1].y = src_size.height;
	blit_region.srcOffsets[1].z = 1;

	blit_region.dstOffsets[1].x = dst_size.width;
	blit_region.dstOffsets[1].y = dst_size.height;
	blit_region.dstOffsets[1].z = 1;

	blit_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blit_region.srcSubresource.baseArrayLayer = 0;
	blit_region.srcSubresource.layerCount = 1;
	blit_region.srcSubresource.mipLevel = 0;

	blit_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blit_region.dstSubresource.baseArrayLayer = 0;
	blit_region.dstSubresource.layerCount = 1;
	blit_region.dstSubresource.mipLevel = 0;

	VkBlitImageInfo2 blit_info{ .sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2,
		.pNext = nullptr };
	blit_info.dstImage = destination;
	blit_info.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	blit_info.srcImage = source;
	blit_info.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	blit_info.filter = VK_FILTER_LINEAR;
	blit_info.regionCount = 1;
	blit_info.pRegions = &blit_region;

	vkCmdBlitImage2(cmd, &blit_info);
}
