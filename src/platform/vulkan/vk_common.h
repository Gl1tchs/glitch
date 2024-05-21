#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan.h>

#include "platform/vulkan/vk_context.h"

#include "gl/renderer/vertex.h"

#define VK_CHECK(x)                                                            \
	do {                                                                       \
		VkResult err = x;                                                      \
		if (err) {                                                             \
			GL_LOG_ERROR("Detected Vulkan error: {}", string_VkResult(err));   \
			GL_ASSERT(false);                                                  \
		}                                                                      \
	} while (false)

struct DrawPushConstants {
	// Mat4f world_matrix;
	VkDeviceAddress vertex_buffer;
};
