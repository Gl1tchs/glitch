#include "platform/vulkan/vk_backend.h"

static BitField<VkImageUsageFlags> _gl_to_vk_image_usage_flags(
		BitField<ImageUsageBits> p_usage) {
	BitField<VkImageUsageFlags> vk_usage;
	if (p_usage.has_flag(IMAGE_USAGE_TRANSFER_SRC_BIT)) {
		vk_usage.set_flag(VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
	}
	if (p_usage.has_flag(IMAGE_USAGE_TRANSFER_DST_BIT)) {
		vk_usage.set_flag(VK_IMAGE_USAGE_TRANSFER_DST_BIT);
	}
	if (p_usage.has_flag(IMAGE_USAGE_SAMPLED_BIT)) {
		vk_usage.set_flag(VK_IMAGE_USAGE_SAMPLED_BIT);
	}
	if (p_usage.has_flag(IMAGE_USAGE_STORAGE_BIT)) {
		vk_usage.set_flag(VK_IMAGE_USAGE_STORAGE_BIT);
	}
	if (p_usage.has_flag(IMAGE_USAGE_COLOR_ATTACHMENT_BIT)) {
		vk_usage.set_flag(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
	}
	if (p_usage.has_flag(IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
		vk_usage.set_flag(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}

	return vk_usage;
}

Image VulkanRenderBackend::_image_create(VkFormat p_format, VkExtent3D p_size,
		BitField<VkImageUsageFlags> p_usage, bool p_mipmapped) {
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
	VK_CHECK(vmaCreateImage(allocator, &img_info, &alloc_info, &vk_image,
			&vma_allocation, nullptr));

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
	VK_CHECK(vkCreateImageView(device, &view_info, nullptr, &vk_image_view));

	// Bookkeep
	VulkanImage* image =
			VersatileResource::allocate<VulkanImage>(resources_allocator);
	image->vk_image = vk_image;
	image->vk_image_view = vk_image_view;
	image->allocation = vma_allocation;
	image->image_format = p_format;
	image->image_extent = p_size;

	return Image(image);
}

Image VulkanRenderBackend::image_create(DataFormat p_format, Vec2u p_size,
		const void* p_data, BitField<ImageUsageBits> p_usage,
		bool p_mipmapped) {
	VkExtent3D vk_size = { p_size.x, p_size.y, 1 };
	VkFormat vk_format = static_cast<VkFormat>(p_format);

	BitField<VkImageUsageFlags> vk_usage = _gl_to_vk_image_usage_flags(p_usage);

	if (!p_data) {
		return _image_create(vk_format, vk_size, vk_usage, p_mipmapped);
	} else {
		const size_t data_size =
				vk_size.depth * vk_size.width * vk_size.height * 4;

		Buffer staging_buffer = buffer_create(data_size,
				BUFFER_USAGE_TRANSFER_SRC_BIT, MEMORY_ALLOCATION_TYPE_CPU);

		uint8_t* mapped_data = buffer_map(staging_buffer);
		{ memcpy(mapped_data, p_data, data_size); }
		buffer_unmap(staging_buffer);

		VkImageUsageFlags image_usage = vk_usage;
		image_usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		image_usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

		Image new_image =
				_image_create(vk_format, vk_size, image_usage, p_mipmapped);

		command_immediate_submit([&](CommandBuffer cmd) {
			command_transition_image(cmd, new_image, IMAGE_LAYOUT_UNDEFINED,
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
			command_copy_buffer_to_image(
					cmd, staging_buffer, new_image, copy_view);

			command_transition_image(cmd, new_image,
					IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		});

		buffer_free(staging_buffer);

		return Image(new_image);
	}
}

void VulkanRenderBackend::image_free(Image p_image) {
	VulkanImage* image = (VulkanImage*)p_image;

	vkDestroyImageView(device, image->vk_image_view, nullptr);
	vmaDestroyImage(allocator, image->vk_image, image->allocation);
}

Vec3u VulkanRenderBackend::image_get_size(Image p_image) {
	VulkanImage* image = (VulkanImage*)p_image;

	Vec3u size;
	static_assert(sizeof(Vec3u) == sizeof(VkExtent3D));
	memcpy(&size, &image->image_extent, sizeof(Vec3u));

	return size;
}

Sampler VulkanRenderBackend::sampler_create(ImageFiltering p_filtering) {
	VkSamplerCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	create_info.minFilter = static_cast<VkFilter>(p_filtering);
	create_info.magFilter = static_cast<VkFilter>(p_filtering);
	// TODO other fields

	VkSampler vk_sampler = VK_NULL_HANDLE;
	VK_CHECK(vkCreateSampler(device, &create_info, nullptr, &vk_sampler));

	return Sampler(vk_sampler);
}

void VulkanRenderBackend::sampler_free(Sampler p_sampler) {
	vkDestroySampler(device, (VkSampler)p_sampler, nullptr);
}
