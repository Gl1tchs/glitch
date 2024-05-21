#include "gl/renderer/image.h"

#include "gl/renderer/renderer.h"

#include "platform/vulkan/vk_image.h"
#include "platform/vulkan/vk_renderer.h"

Ref<Image> Image::create(Vec2u size, ImageFormat format, bool mipmapped) {
	switch (Renderer::get_backend()) {
		case RenderBackend::Vulkan: {
			Ref<VulkanImage> vk_image =
					VulkanImage::create(VulkanRenderer::get_context(),
							VkExtent3D{ size.x, size.y, 1 },
							image_format_to_vk_format(format),
							VK_IMAGE_USAGE_SAMPLED_BIT, mipmapped);

			return vk_image;
		}

		default: {
			return nullptr;
		}
	}
}

Ref<Image> Image::create(
		const void* data, Vec2u size, ImageFormat format, bool mipmapped) {
	switch (Renderer::get_backend()) {
		case RenderBackend::Vulkan: {
			Ref<VulkanImage> vk_image =
					VulkanImage::create(VulkanRenderer::get_context(), data,
							VkExtent3D{ size.x, size.y, 1 },
							image_format_to_vk_format(format),
							VK_IMAGE_USAGE_SAMPLED_BIT, mipmapped);

			return vk_image;
		}

		default: {
			return nullptr;
		}
	}
}

void Image::destroy(Ref<Image> image) {
	switch (Renderer::get_backend()) {
		case RenderBackend::Vulkan: {
			Ref<VulkanImage> vk_image =
					std::dynamic_pointer_cast<VulkanImage>(image);
			VulkanImage::destroy(VulkanRenderer::get_context(), vk_image.get());

			break;
		}
		default: {
			break;
		}
	}
}
