#include "glitch/platform/vulkan/vk_backend.h"

namespace gl {

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
		BitField<VkImageUsageFlags> p_usage, bool p_mipmapped,
		VkSampleCountFlagBits p_samples) {
	const uint32_t mip_levels = p_mipmapped
			? static_cast<uint32_t>(std::floor(
					  std::log2(std::max(p_size.width, p_size.height)))) +
					1
			: 1;

	VkImageCreateInfo img_info = {};
	img_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	img_info.pNext = nullptr;
	img_info.imageType = VK_IMAGE_TYPE_2D;
	img_info.format = p_format;
	img_info.extent = p_size;
	// Set mipmap levels
	img_info.mipLevels = mip_levels;
	img_info.arrayLayers = 1;
	// MSAA
	img_info.samples = p_samples;
	// optimal tiling, which means the image is stored on the best gpu format
	img_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	img_info.usage = p_usage;

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
	view_info.subresourceRange.levelCount = mip_levels;
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
	image->image_extent = p_size;
	image->image_format = p_format;
	image->mip_levels = mip_levels;

	return Image(image);
}

void VulkanRenderBackend::_generate_image_mipmaps(
		CommandBuffer p_cmd, Image p_image, glm::uvec2 p_size) {
	const uint32_t mip_levels = image_get_mip_levels(p_image);
	int mip_width = p_size.x;
	int mip_height = p_size.y;
	for (uint32_t i = 1; i < mip_levels; i++) {
		command_transition_image(p_cmd, p_image,
				ImageLayout::TRANSFER_DST_OPTIMAL,
				ImageLayout::TRANSFER_SRC_OPTIMAL, i - 1, 1);

		command_copy_image_to_image(p_cmd, p_image, p_image,
				{ mip_width, mip_height },
				{ mip_width > 1 ? mip_width / 2 : 1,
						mip_height > 1 ? mip_height / 2 : 1 },
				i - 1, i);

		command_transition_image(p_cmd, p_image,
				ImageLayout::TRANSFER_SRC_OPTIMAL,
				ImageLayout::SHADER_READ_ONLY_OPTIMAL, i - 1, 1);

		if (mip_width > 1) {
			mip_width /= 2;
		}
		if (mip_height > 1) {
			mip_height /= 2;
		}
	}

	command_transition_image(p_cmd, p_image, ImageLayout::TRANSFER_DST_OPTIMAL,
			ImageLayout::SHADER_READ_ONLY_OPTIMAL, mip_levels - 1, 1);
}

Image VulkanRenderBackend::image_create(DataFormat p_format, glm::uvec2 p_size,
		const void* p_data, BitField<ImageUsageBits> p_usage, bool p_mipmapped,
		uint32_t p_samples) {
	VkExtent3D vk_size = { p_size.x, p_size.y, 1 };
	VkFormat vk_format = static_cast<VkFormat>(p_format);

	BitField<VkImageUsageFlags> vk_usage = _gl_to_vk_image_usage_flags(p_usage);

	if (!p_data) {
		return _image_create(vk_format, vk_size, vk_usage, p_mipmapped,
				static_cast<VkSampleCountFlagBits>(p_samples));
	} else {
		const size_t data_size =
				vk_size.depth * vk_size.width * vk_size.height * 4;

		Buffer staging_buffer = buffer_create(data_size,
				BUFFER_USAGE_TRANSFER_SRC_BIT, MemoryAllocationType::CPU);

		uint8_t* mapped_data = buffer_map(staging_buffer);
		{
			memcpy(mapped_data, p_data, data_size);
		}
		buffer_unmap(staging_buffer);

		VkImageUsageFlags image_usage = vk_usage;
		image_usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		image_usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

		Image new_image = _image_create(vk_format, vk_size, image_usage,
				p_mipmapped, static_cast<VkSampleCountFlagBits>(p_samples));

		command_immediate_submit(
				[&](CommandBuffer p_cmd) {
					command_transition_image(p_cmd, new_image,
							ImageLayout::UNDEFINED,
							ImageLayout::TRANSFER_DST_OPTIMAL);

					BufferImageCopyRegion copy_region = {};
					copy_region.buffer_offset = 0;
					copy_region.buffer_row_length = 0;
					copy_region.buffer_image_height = 0;
					copy_region.image_subresource = {};
					copy_region.image_subresource.aspect_mask =
							IMAGE_ASPECT_COLOR_BIT;
					copy_region.image_subresource.mip_level = 0;
					copy_region.image_subresource.base_array_layer = 0;
					copy_region.image_subresource.layer_count = 1;
					copy_region.image_extent = { p_size.x, p_size.y, 1 };
					copy_region.image_offset = { 0, 0, 0 };

					VectorView<BufferImageCopyRegion> copy_view(copy_region);

					// copy the buffer into the image
					command_copy_buffer_to_image(
							p_cmd, staging_buffer, new_image, copy_view);

					// generate mipmaps
					if (p_mipmapped) {
						_generate_image_mipmaps(p_cmd, new_image, p_size);
					} else {
						command_transition_image(p_cmd, new_image,
								ImageLayout::TRANSFER_DST_OPTIMAL,
								ImageLayout::SHADER_READ_ONLY_OPTIMAL);
					}
				},
				QueueType::GRAPHICS);

		buffer_free(staging_buffer);

		return Image(new_image);
	}
}

void VulkanRenderBackend::image_free(Image p_image) {
	VulkanImage* image = (VulkanImage*)p_image;

	vkDestroyImageView(device, image->vk_image_view, nullptr);
	vmaDestroyImage(allocator, image->vk_image, image->allocation);
}

glm::uvec3 VulkanRenderBackend::image_get_size(Image p_image) {
	VulkanImage* image = (VulkanImage*)p_image;

	glm::uvec3 size;
	static_assert(sizeof(glm::uvec3) == sizeof(VkExtent3D));
	memcpy(&size, &image->image_extent, sizeof(glm::uvec3));

	return size;
}

DataFormat VulkanRenderBackend::image_get_format(Image p_image) {
	VulkanImage* image = (VulkanImage*)p_image;
	return static_cast<DataFormat>(image->image_format);
}

uint32_t VulkanRenderBackend::image_get_mip_levels(Image p_image) {
	VulkanImage* image = (VulkanImage*)p_image;
	return image->mip_levels;
}

Sampler VulkanRenderBackend::sampler_create(ImageFiltering p_min_filter,
		ImageFiltering p_mag_filter, ImageWrappingMode p_wrap_u,
		ImageWrappingMode p_wrap_v, ImageWrappingMode p_wrap_w,
		uint32_t p_mip_levels) {
	VkSamplerCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	create_info.minFilter = static_cast<VkFilter>(p_min_filter);
	create_info.magFilter = static_cast<VkFilter>(p_mag_filter);

	create_info.addressModeU = static_cast<VkSamplerAddressMode>(p_wrap_u);
	create_info.addressModeV = static_cast<VkSamplerAddressMode>(p_wrap_v);
	create_info.addressModeW = static_cast<VkSamplerAddressMode>(p_wrap_w);

	if (p_mip_levels > 0) {
		create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		create_info.minLod = 0.0f;
		create_info.maxLod = static_cast<float>(p_mip_levels);
		create_info.mipLodBias = 0.0f;
	}

	VkSampler vk_sampler = VK_NULL_HANDLE;
	VK_CHECK(vkCreateSampler(device, &create_info, nullptr, &vk_sampler));

	return Sampler(vk_sampler);
}

void VulkanRenderBackend::sampler_free(Sampler p_sampler) {
	vkDestroySampler(device, (VkSampler)p_sampler, nullptr);
}

} //namespace gl