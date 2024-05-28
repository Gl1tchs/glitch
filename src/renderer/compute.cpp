#include "gl/renderer/compute.h"

#include "gl/renderer/renderer.h"

#include "platform/vulkan/vk_compute.h"
#include "platform/vulkan/vk_renderer.h"

Ref<ComputeEffect> ComputeEffect::create(const ComputeEffectCreateInfo* info) {
	switch (Renderer::get_backend()) {
		case RenderBackend::Vulkan: {
			VulkanComputeEffectCreateInfo vk_info = {
				.group_count_x = info->group_count_x,
				.group_count_y = info->group_count_y,
				.group_count_z = info->group_count_z,
				.shader_spv_path = info->shader_spv_path,
			};
			Ref<VulkanComputeEffect> vk_effect = VulkanComputeEffect::create(
					VulkanRenderer::get_context(), &vk_info);

			return vk_effect;
		}

		default: {
			return nullptr;
		}
	}
}

void ComputeEffect::destroy(Ref<ComputeEffect> effect) {
	switch (Renderer::get_backend()) {
		case RenderBackend::Vulkan: {
			Ref<VulkanComputeEffect> vk_effect =
					std::dynamic_pointer_cast<VulkanComputeEffect>(effect);
			VulkanComputeEffect::destroy(
					VulkanRenderer::get_context(), vk_effect.get());
			break;
		}

		default: {
			break;
		}
	}
}
