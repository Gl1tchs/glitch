#include "gl/renderer/compute.h"

#include "gl/renderer/renderer.h"

#include "platform/vulkan/vk_compute.h"
#include "platform/vulkan/vk_renderer.h"

Ref<ComputeEffect> ComputeEffect::create(const ComputeEffectCreateInfo* info) {
	switch (Renderer::get_backend()) {
		case RenderBackend::Vulkan: {
			VulkanComputeEffectCreateInfo vk_info = {
				.shader_spv_path = info->shader_spv_path,
				.group_count = info->group_count,
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
