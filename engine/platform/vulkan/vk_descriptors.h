#pragma once

#include "platform/vulkan/vk_common.h"

#include <span>

struct DescriptorLayoutBuilder {
	std::vector<VkDescriptorSetLayoutBinding> bindings;

	void add_binding(uint32_t binding, VkDescriptorType type);

	void clear();

	VkDescriptorSetLayout build(VkDevice device,
			VkShaderStageFlags shader_stages, void* next = nullptr,
			VkDescriptorSetLayoutCreateFlags flags = 0);
};

struct VulkanDescriptorAllocator {
	struct PoolSizeRatio {
		VkDescriptorType type;
		float ratio;
	};

	void init(VkDevice device, uint32_t initial_sets,
			std::span<PoolSizeRatio> pool_size_ratios);

	void clear_pools(VkDevice device);

	void destroy_pools(VkDevice device);

	VkDescriptorSet allocate(VkDevice device, VkDescriptorSetLayout layout,
			void* next = nullptr);

private:
	VkDescriptorPool get_pool(VkDevice device);
	VkDescriptorPool create_pool(VkDevice device, uint32_t set_count,
			std::span<PoolSizeRatio> pool_ratios);

	std::vector<PoolSizeRatio> ratios;
	std::vector<VkDescriptorPool> full_pools;
	std::vector<VkDescriptorPool> ready_pools;
	uint32_t sets_per_pool;
};

struct DescriptorWriter {
	std::deque<VkDescriptorImageInfo> image_infos;
	std::deque<VkDescriptorBufferInfo> buffer_infos;
	std::vector<VkWriteDescriptorSet> writes;

	// In both the write_image and write_buffer functions, we are being overly
	// generic. This is done for simplicity, but if you want, you can add new
	// ones like write_sampler() where it has VK_DESCRIPTOR_TYPE_SAMPLER and
	// sets imageview and layout to null, and other similar abstractions.

	void write_image(int binding, VkImageView image, VkSampler sampler,
			VkImageLayout layout, VkDescriptorType type);
	void write_buffer(int binding, VkBuffer buffer, size_t size, size_t offset,
			VkDescriptorType type);

	void clear();

	void update_set(VkDevice device, VkDescriptorSet set);
};
