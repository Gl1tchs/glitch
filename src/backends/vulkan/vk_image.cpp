#include "backends/vulkan/vk_image.h"

#include "backends/vulkan/vk_buffer.h"
#include "backends/vulkan/vk_commands.h"
#include "backends/vulkan/vk_context.h"

#include <vulkan/vulkan_core.h>

namespace vk {

static Image _image_create(VulkanContext* p_context, VkFormat p_format,
		VkExtent3D p_size, VkImageUsageFlags p_usage, bool p_mipmapped) {
	VkImageCreateInfo img_info = {};
	img_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	img_info.pNext = nullptr;
	img_info.imageType = VK_IMAGE_TYPE_2D;
	img_info.format = p_format;
	img_info.extent = p_size;
	img_info.mipLevels = 1;
	img_info.arrayLayers = 1;
	// for MSAA. we will not be using it by default, so default it to 1 sample
	// per pixel.
	img_info.samples = VK_SAMPLE_COUNT_1_BIT;
	// optimal tiling, which means the image is stored on the best gpu format
	img_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	img_info.usage = p_usage;

	if (p_mipmapped) {
		img_info.mipLevels = static_cast<uint32_t>(std::floor(std::log2(
									 std::max(p_size.width, p_size.height)))) +
				1;
	}

	// always allocate images on dedicated GPU memory
	VmaAllocationCreateInfo alloc_info = {};
	alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	alloc_info.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	alloc_info.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	// allocate and create the image
	VkImage vk_image = VK_NULL_HANDLE;
	VmaAllocation vma_allocation = {};
	VK_CHECK(vmaCreateImage(p_context->allocator, &img_info, &alloc_info,
			&vk_image, &vma_allocation, nullptr));

	// if the format is a depth format, we will need to have it use the correct
	// aspect flag
	VkImageAspectFlags aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT;
	if (img_info.format == VK_FORMAT_D32_SFLOAT) {
		aspect_flags = VK_IMAGE_ASPECT_DEPTH_BIT;
	}

	// build image view for the image
	// build a image-view for the depth image to use for rendering
	VkImageViewCreateInfo view_info = {};
	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.pNext = nullptr;
	view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view_info.image = vk_image;
	view_info.format = p_format;
	view_info.subresourceRange.baseMipLevel = 0;
	view_info.subresourceRange.levelCount = img_info.mipLevels;
	view_info.subresourceRange.baseArrayLayer = 0;
	view_info.subresourceRange.layerCount = 1;
	view_info.subresourceRange.aspectMask = aspect_flags;

	VkImageView vk_image_view = VK_NULL_HANDLE;
	VK_CHECK(vkCreateImageView(
			p_context->device, &view_info, nullptr, &vk_image_view));

	// Bookkeep
	VulkanImage* image = VersatileResource::allocate<VulkanImage>(
			p_context->resources_allocator);
	image->vk_image = vk_image;
	image->vk_image_view = vk_image_view;
	image->allocation = vma_allocation;
	image->image_format = p_format;
	image->image_extent = p_size;

	return Image(image);
}

Image image_create(Context p_context, DataFormat p_format, Vec2u p_size,
		const void* p_data, ImageUsage p_usage, bool p_mipmapped) {
	VulkanContext* vk_context = (VulkanContext*)p_context;

	VkExtent3D vk_size = { p_size.x, p_size.y, 1 };
	VkFormat vk_format = static_cast<VkFormat>(p_format);
	VkImageUsageFlags vk_usage = static_cast<VkImageUsageFlags>(p_usage);

	if (!p_data) {
		return _image_create(
				vk_context, vk_format, vk_size, vk_usage, p_mipmapped);
	} else {
		const size_t data_size =
				vk_size.depth * vk_size.width * vk_size.height * 4;

		Buffer staging_buffer = vk::buffer_create(p_context, data_size,
				BUFFER_USAGE_TRANSFER_FROM_BIT, MEMORY_ALLOCATION_TYPE_CPU);

		uint8_t* mapped_data = vk::buffer_map(p_context, staging_buffer);
		{ memcpy(mapped_data, p_data, data_size); }
		vk::buffer_unmap(p_context, staging_buffer);

		VkImageUsageFlags image_usage = vk_usage;
		image_usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		image_usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

		Image new_image = _image_create(
				vk_context, vk_format, vk_size, image_usage, p_mipmapped);

		vk_context->immediate_submit([&](CommandBuffer cmd) {
			vk::command_transition_image(cmd, new_image, IMAGE_LAYOUT_UNDEFINED,
					IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

			BufferImageCopyRegion copy_region = {};
			copy_region.buffer_offset = 0;
			copy_region.buffer_row_length = 0;
			copy_region.buffer_image_height = 0;
			copy_region.image_subresource = {};
			copy_region.image_subresource.aspect_mask = IMAGE_ASPECT_COLOR_BIT;
			copy_region.image_subresource.mip_level = 0;
			copy_region.image_subresource.base_array_layer = 0;
			copy_region.image_subresource.layer_count = 1;
			copy_region.image_extent = { p_size.x, p_size.y, 1 };
			copy_region.image_offset = 0;

			VectorView<BufferImageCopyRegion> copy_view(copy_region);

			// copy the buffer into the image
			vk::command_copy_buffer_to_image(
					cmd, staging_buffer, new_image, copy_view);

			vk::command_transition_image(cmd, new_image,
					IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		});

		vk::buffer_free(p_context, staging_buffer);

		return Image(new_image);
	}
}

void image_free(Context p_context, Image p_image) {
	VulkanContext* context = (VulkanContext*)p_context;
	VulkanImage* image = (VulkanImage*)p_image;

	vkDestroyImageView(context->device, image->vk_image_view, nullptr);
	vmaDestroyImage(context->allocator, image->vk_image, image->allocation);
}

} //namespace vk