#pragma once

#include "gl/renderer/shader.h"
#include <vulkan/vulkan.h>

struct VulkanShader : public Shader {
	VkShaderModule shader;

	/**
	 * @brief loads shader from bundled engine
	 * shader library
	 */
	static Ref<VulkanShader> get(VkDevice device, const char* file_path);

	/**
	 * @brief loads shader from file
	 */
	static Ref<VulkanShader> create(VkDevice device, const char* file_path);

	/**
	 * @brief loads shader from spirv binary
	 */
	static Ref<VulkanShader> create(
			VkDevice device, uint32_t spirv_size, uint32_t* spirv_data);

	static void destroy(VkDevice device, Ref<VulkanShader> shader);
};
