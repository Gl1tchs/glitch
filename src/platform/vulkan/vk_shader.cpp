#include "platform/vulkan/vk_shader.h"

#include "shader_bundle.gen.h"

Ref<VulkanShader> VulkanShader::get(VkDevice device, const char* file_path) {
	BundleFileData shader_data{};
	bool shader_found = false;

	for (int i = 0; i < BUNDLE_FILE_COUNT; i++) {
		BundleFileData data = BUNDLE_FILES[i];
		if (strcmp(data.path, file_path) == 0) {
			shader_data = data;
			shader_found = true;
			break;
		}
	}

	if (!shader_found) {
		return nullptr;
	}

	// create a new shader module, using the buffer we loaded
	VkShaderModuleCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_info.pNext = nullptr;

	// codeSize has to be in bytes, so multiply the ints in the buffer by size
	// of int to know the real size of the buffer
	create_info.codeSize = shader_data.size;
	create_info.pCode = (uint32_t*)&BUNDLE_DATA[shader_data.start_idx];

	Ref<VulkanShader> shader = create_ref<VulkanShader>();

	// check that the creation goes well.
	if (vkCreateShaderModule(device, &create_info, nullptr, &shader->shader) !=
			VK_SUCCESS) {
		return nullptr;
	}

	return shader;
}

Ref<VulkanShader> VulkanShader::create(VkDevice device, const char* file_path) {
	std::ifstream file(file_path, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		return nullptr;
	}

	size_t file_size = (size_t)file.tellg();

	std::vector<uint32_t> buffer(file_size / sizeof(uint32_t));

	// put file cursor at beginning
	file.seekg(0);

	// load the entire file into the buffer
	file.read((char*)buffer.data(), file_size);

	file.close();

	return VulkanShader::create(
			device, buffer.size() * sizeof(uint32_t), buffer.data());
}

Ref<VulkanShader> VulkanShader::create(
		VkDevice device, uint32_t spirv_size, uint32_t* spirv_data) {
	// create a new shader module, using the buffer we loaded
	VkShaderModuleCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_info.pNext = nullptr;

	// codeSize has to be in bytes, so multiply the ints in the buffer by size
	// of int to know the real size of the buffer
	create_info.codeSize = spirv_size;
	create_info.pCode = spirv_data;

	Ref<VulkanShader> shader = create_ref<VulkanShader>();

	// check that the creation goes well.
	if (vkCreateShaderModule(device, &create_info, nullptr, &shader->shader) !=
			VK_SUCCESS) {
		return nullptr;
	}

	return shader;
}

void VulkanShader::destroy(VkDevice device, Ref<VulkanShader> shader) {
	vkDestroyShaderModule(device, shader->shader, nullptr);
}
