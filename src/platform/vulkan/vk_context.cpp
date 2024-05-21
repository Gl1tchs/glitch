#include "platform/vulkan/vk_context.h"

VkSampler VulkanContext::get_sampler(ImageFilteringMode mode) const {
	switch (mode) {
		case ImageFilteringMode::LINEAR:
			return linear_sampler;
		case ImageFilteringMode::NEAREST:
			return nearest_sampler;
		default:
			return linear_sampler;
	}
}
