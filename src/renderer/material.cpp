#include "gl/renderer/material.h"

#include "gl/renderer/renderer.h"

#include "platform/vulkan/vk_material.h"
#include "platform/vulkan/vk_renderer.h"

Ref<MetallicRoughnessMaterial> MetallicRoughnessMaterial::create() {
	switch (Renderer::get_backend()) {
		case RenderBackend::Vulkan: {
			Ref<VulkanMetallicRoughnessMaterial> vk_material =
					VulkanMetallicRoughnessMaterial::create(
							VulkanRenderer::get_context());

			return vk_material;
		}

		default: {
			return nullptr;
		}
	}
}

void MetallicRoughnessMaterial::destroy(
		Ref<MetallicRoughnessMaterial> material) {
	switch (Renderer::get_backend()) {
		case RenderBackend::Vulkan: {
			Ref<VulkanMetallicRoughnessMaterial> vk_material =
					std::dynamic_pointer_cast<VulkanMetallicRoughnessMaterial>(
							material);
			VulkanMetallicRoughnessMaterial::destroy(
					VulkanRenderer::get_context(), vk_material.get());
		}

		default: {
			break;
		}
	}
}

Ref<MaterialInstance> MetallicRoughnessMaterial::create_instance(
		const MaterialResources& resources) {
	switch (Renderer::get_backend()) {
		case RenderBackend::Vulkan: {
			VulkanMetallicRoughnessMaterial* vk_material =
					reinterpret_cast<VulkanMetallicRoughnessMaterial*>(this);

			Ref<VulkanMaterialInstance> vk_instance =
					vk_material->create_instance(
							VulkanRenderer::get_context(), resources);

			return vk_instance;
		}
		default: {
			return nullptr;
		}
	}
}
