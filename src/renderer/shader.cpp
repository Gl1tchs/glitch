#include "gl/renderer/shader.h"
#include "gl/renderer/renderer.h"

#include "platform/vulkan/vk_renderer.h"
#include "platform/vulkan/vk_shader.h"

Ref<Shader> Shader::create(const char* file_path) {
	switch (Renderer::get_backend()) {
		case RenderBackend::Vulkan: {
			Ref<VulkanShader> vk_shader = VulkanShader::create(
					VulkanRenderer::get_context().device, file_path);

			return vk_shader;
		}

		default: {
			return nullptr;
		}
	}
}

Ref<Shader> Shader::create(uint32_t spirv_size, uint32_t* spirv_data) {
	switch (Renderer::get_backend()) {
		case RenderBackend::Vulkan: {
			Ref<VulkanShader> vk_shader =
					VulkanShader::create(VulkanRenderer::get_context().device,
							spirv_size, spirv_data);

			return vk_shader;
		}

		default: {
			return nullptr;
		}
	}
}

void Shader::destroy(Ref<Shader> shader) {
	switch (Renderer::get_backend()) {
		case RenderBackend::Vulkan: {
			Ref<VulkanShader> vk_shader =
					std::dynamic_pointer_cast<VulkanShader>(shader);
			VulkanShader::destroy(
					VulkanRenderer::get_context().device, vk_shader);
			break;
		}

		default: {
			break;
		}
	}
}
