#include "renderer/mesh.h"

#include "renderer/renderer.h"

#include "platform/vulkan/vk_mesh.h"
#include "platform/vulkan/vk_renderer.h"

Ref<Mesh> Mesh::create(
		const std::span<Vertex> vertices, const std::span<uint32_t> indices) {
	switch (Renderer::get_backend()) {
		case RenderBackend::Vulkan: {
			Ref<VulkanMesh> vk_mesh = VulkanMesh::create(
					VulkanRenderer::get_context(), vertices, indices);

			return vk_mesh;
		}

		default: {
			return nullptr;
		}
	}
}

void Mesh::destroy(Ref<Mesh> mesh) {
	switch (Renderer::get_backend()) {
		case RenderBackend::Vulkan: {
			Ref<VulkanMesh> vk_mesh =
					std::dynamic_pointer_cast<VulkanMesh>(mesh);
			VulkanMesh::destroy(VulkanRenderer::get_context(), vk_mesh.get());

			break;
		}
		default: {
			break;
		}
	}
}
