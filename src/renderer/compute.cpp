#include "gl/renderer/compute.h"

#include "gl/renderer/renderer.h"

#include "platform/vulkan/vk_compute.h"
#include "platform/vulkan/vk_renderer.h"

Ref<ComputeEffectNode> ComputeEffectNode::create(
		const ComputeEffectCreateInfo* info) {
	switch (Renderer::get_backend()) {
		case RenderBackend::Vulkan: {
			VulkanComputeEffectCreateInfo vk_info = {
				.shader_spv_path = info->shader_spv_path,
				.group_count = info->group_count,
			};
			Ref<VulkanComputeEffectNode> vk_effect =
					VulkanComputeEffectNode::create(
							VulkanRenderer::get_context(), &vk_info);

			return vk_effect;
		}

		default: {
			return nullptr;
		}
	}
}

void ComputeEffectNode::destroy(const ComputeEffectNode* effect) {
	switch (Renderer::get_backend()) {
		case RenderBackend::Vulkan: {
			const VulkanComputeEffectNode* vk_effect =
					reinterpret_cast<const VulkanComputeEffectNode*>(effect);
			VulkanComputeEffectNode::destroy(
					VulkanRenderer::get_context(), vk_effect);
			break;
		}

		default: {
			break;
		}
	}
}
