#include "renderer/render_context.h"

#include "renderer/vulkan/vk_context.h"

[[nodiscard]] RenderBackend find_proper_backend() noexcept {
	// we don't have any other backend thus hardcode vulkan
	return RenderBackend::Vulkan;
}

void RenderContext::init(Ref<Window> window) {
	switch (backend) {
		case RenderBackend::Vulkan: {
			// create and initialize vulkan context
			auto context = new VulkanContext();
			context->init(window);

			// store the pointer
			native_context = context;

			break;
		}
		default:
			break;
	}
}

void RenderContext::destroy() {
	switch (backend) {
		case RenderBackend::Vulkan: {
			auto context = (VulkanContext*)native_context;
			context->destroy();

			// destroy the pointer
			delete context;

			break;
		}
		default:
			break;
	}
}
