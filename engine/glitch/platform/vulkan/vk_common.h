#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#define VK_CHECK(x)                                                            \
	do {                                                                       \
		VkResult err = x;                                                      \
		if (err) {                                                             \
			GL_LOG_ERROR(                                                      \
					"Detected Vulkan error: {}", vk_result_to_string(err));    \
			GL_ASSERT(false);                                                  \
		}                                                                      \
	} while (false)

namespace gl {

GL_API const char* vk_result_to_string(VkResult res);

}
