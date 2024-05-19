#include "renderer/mesh.h"

#include "platform/vulkan/vk_renderer.h"
#include "renderer/renderer.h"

#include "platform/vulkan/vk_mesh.h"

Mesh::~Mesh() {
	switch (Renderer::get_backend()) {
		case RenderBackend::Vulkan: {
			VulkanMesh* vk_mesh = reinterpret_cast<VulkanMesh*>(this);
			VulkanMesh::destroy(VulkanRenderer::get_context(), *vk_mesh);

			break;
		}
		default: {
			break;
		}
	}
}

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
