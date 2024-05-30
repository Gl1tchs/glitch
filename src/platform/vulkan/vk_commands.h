#pragma once

#include "platform/vulkan/vk_buffer.h"
#include "platform/vulkan/vk_compute.h"
#include "platform/vulkan/vk_image.h"
#include "platform/vulkan/vk_pipeline.h"

struct VulkanCommandBuffer;

struct VulkanCommandPool {
	VkCommandPool command_pool;

	static VulkanCommandPool create(
			VkDevice device, const VkCommandPoolCreateInfo* info);
	static VulkanCommandPool create(VkDevice device,
			uint32_t queue_family_index, VkCommandPoolCreateFlags flags = 0);

	static void destroy(VkDevice device, VulkanCommandPool& command_pool);

	VulkanCommandBuffer allocate_buffer(VkDevice device);

	std::vector<VulkanCommandBuffer> allocate_buffer(
			VkDevice device, const uint32_t count);

	void reset(VkDevice device);
};

struct VulkanCommandBuffer {
	VkCommandBuffer command_buffer;

	void begin(VkBufferUsageFlags flags = 0);

	void end();

	void reset(VkCommandBufferResetFlags flags = 0);

	void submit(VkQueue queue, VkFence fence,
			const VkSemaphoreSubmitInfo* wait_semaphore = nullptr,
			const VkSemaphoreSubmitInfo* signal_semaphore = nullptr);

	void begin_rendering(VkExtent2D draw_extent,
			uint32_t color_attachment_count,
			const VkRenderingAttachmentInfo* color_attachments,
			const VkRenderingAttachmentInfo* depth_attachment);

	void end_rendering();

	void bind_pipeline(const VulkanPipeline& pipeline);

	void bind_pipeline(const VulkanComputePipeline& pipeline);

	void bind_index_buffer(const VulkanBuffer& index_buffer,
			VkDeviceSize offset, VkIndexType index_type);

	void draw_indexed(uint32_t index_count, uint32_t instance_count = 1,
			uint32_t first_index = 0, int32_t vertex_offset = 0,
			uint32_t first_instance = 0);

	void dispatch(uint32_t group_count_x, uint32_t group_count_y,
			uint32_t group_count_z);

	void bind_descriptor_sets(const VulkanPipelineLayout& pipeline_layout,
			uint32_t first_set, uint32_t descriptor_set_count,
			const VkDescriptorSet* descriptor_sets,
			VkPipelineBindPoint bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS);

	void push_constants(const VulkanPipelineLayout& pipeline_layout,
			VkShaderStageFlags shader_stages, VkDeviceSize offset,
			uint32_t size, const void* push_constants);

	void set_viewport(Vec2f size);

	void set_scissor(VkExtent2D size, VkOffset2D offset = { 0, 0 });

	void clear_color_image(VkImage image, VkImageLayout image_layout,
			const VkClearColorValue* clear_color,
			VkImageAspectFlags aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT);

	void copy_buffer(const VulkanBuffer& src_buffer, VulkanBuffer& dst_buffer,
			uint32_t region_count, const VkBufferCopy* regions);

	void copy_buffer_to_image(const VulkanBuffer& src_buffer,
			Ref<VulkanImage> dst_image, VkImageLayout image_layout,
			uint32_t region_count, const VkBufferImageCopy* regions);

	void copy_image_to_image(
			Ref<VulkanImage> src_image, Ref<VulkanImage> dst_image);
	void copy_image_to_image(VkImage src_image, VkImage dst_image,
			const VkExtent2D& src_extent, const VkExtent2D& dst_extent);

	void transition_image(Ref<VulkanImage> image, VkImageLayout current_layout,
			VkImageLayout new_layout);
	void transition_image(VkImage image, VkImageLayout current_layout,
			VkImageLayout new_layout);
};
