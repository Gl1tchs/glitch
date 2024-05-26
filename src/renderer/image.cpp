#include "gl/renderer/image.h"

#include "gl/renderer/renderer.h"

#include "platform/vulkan/vk_image.h"
#include "platform/vulkan/vk_renderer.h"

Ref<Image> Image::create(const ImageCreateInfo* info) {
	switch (Renderer::get_backend()) {
		case RenderBackend::Vulkan: {
			VulkanImageCreateInfo vk_info = {
				.format = image_format_to_vk_format(info->format),
				.size = VkExtent3D{ info->size.x, info->size.y, 1 },
				.data = info->data,
				.usage = VK_IMAGE_USAGE_SAMPLED_BIT,
				.mipmapped = info->mipmapped,
			};

			Ref<VulkanImage> vk_image = VulkanImage::create(
					VulkanRenderer::get_context(), &vk_info);

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
