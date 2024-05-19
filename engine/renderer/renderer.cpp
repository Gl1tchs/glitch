#include "renderer/renderer.h"

#include "platform/vulkan/vk_renderer.h"

[[nodiscard]] RenderBackend find_proper_backend() noexcept {
	return RenderBackend::Vulkan;
}

static RenderBackend s_backend;

Ref<Renderer> Renderer::create(RenderBackend backend, Ref<Window> window) {
	s_backend = backend;

	switch (backend) {
		case RenderBackend::Vulkan: {
			// create and initialize vulkan renderer
			auto context = create_ref<VulkanRenderer>(window);

			return context;
		}
		default: {
			return nullptr;
		}
	}
}

RenderBackend Renderer::get_backend() { return s_backend; }
