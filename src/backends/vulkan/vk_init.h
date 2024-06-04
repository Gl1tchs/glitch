#pragma once

#include "backends/vulkan/vk_common.h"

namespace vkinit {

VkCommandPoolCreateInfo command_pool_create_info(
		uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0);

VkCommandBufferAllocateInfo command_buffer_allocate_info(
		VkCommandPool pool, uint32_t count = 1);

VkFenceCreateInfo fence_create_info(VkFenceCreateFlags flags = 0);

VkSemaphoreCreateInfo semaphore_create_info(VkSemaphoreCreateFlags flags = 0);

VkCommandBufferBeginInfo command_buffer_begin_info(
		VkCommandBufferUsageFlags flags = 0);

VkImageSubresourceRange image_subresource_range(VkImageAspectFlags aspect_mask);

VkSemaphoreSubmitInfo semaphore_submit_info(
		VkPipelineStageFlags2 stage_mask, VkSemaphore semaphore);

VkCommandBufferSubmitInfo command_buffer_submit_info(VkCommandBuffer cmd);

VkSubmitInfo2 submit_info(VkCommandBufferSubmitInfo* cmd,
		const VkSemaphoreSubmitInfo* wait_semaphore_info = nullptr,
		const VkSemaphoreSubmitInfo* signal_semaphore_info = nullptr);

VkImageCreateInfo image_create_info(
		VkFormat format, VkImageUsageFlags usage_flags, VkExtent3D extent);

VkImageViewCreateInfo imageview_create_info(
		VkFormat format, VkImage image, VkImageAspectFlags aspect_flags);

VkRenderingAttachmentInfo depth_attachment_info(VkImageView view,
		VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

VkRenderingAttachmentInfo attachment_info(VkImageView view,
		const VkClearValue* clear,
		VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

VkPipelineLayoutCreateInfo pipeline_layout_create_info();

VkPipelineShaderStageCreateInfo pipeline_shader_stage_create_info(
		VkShaderStageFlagBits stage, VkShaderModule shader_module,
		const char* entry = "main");

} //namespace vkinit
