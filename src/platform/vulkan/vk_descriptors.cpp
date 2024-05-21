#include "platform/vulkan/vk_descriptors.h"

#include "platform/vulkan/vk_common.h"

void DescriptorLayoutBuilder::add_binding(
		uint32_t binding, VkDescriptorType type) {
	VkDescriptorSetLayoutBinding new_binding{};
	new_binding.binding = binding;
	new_binding.descriptorCount = 1;
	new_binding.descriptorType = type;

	bindings.push_back(new_binding);
}

void DescriptorLayoutBuilder::clear() { bindings.clear(); }

VkDescriptorSetLayout DescriptorLayoutBuilder::build(VkDevice device,
		VkShaderStageFlags shader_stages, void* next,
		VkDescriptorSetLayoutCreateFlags flags) {
	for (auto& b : bindings) {
		b.stageFlags |= shader_stages;
	}

	VkDescriptorSetLayoutCreateInfo info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.pNext = next,
		.flags = flags,
		.bindingCount = (uint32_t)bindings.size(),
		.pBindings = bindings.data(),
	};

	VkDescriptorSetLayout set;
	VK_CHECK(vkCreateDescriptorSetLayout(device, &info, nullptr, &set));

	return set;
}

void VulkanDescriptorAllocator::init(VkDevice device, uint32_t initial_sets,
		std::span<PoolSizeRatio> pool_size_ratios) {
	ratios.clear();

	// TODO use generic function
	for (auto r : pool_size_ratios) {
		ratios.push_back(r);
	}

	VkDescriptorPool new_pool =
			create_pool(device, initial_sets, pool_size_ratios);

	sets_per_pool = initial_sets * 1.5f;

	ready_pools.push_back(new_pool);
}

void VulkanDescriptorAllocator::clear_pools(VkDevice device) {
	for (auto p : ready_pools) {
		vkResetDescriptorPool(device, p, 0);
	}
	for (auto p : full_pools) {
		vkResetDescriptorPool(device, p, 0);
		ready_pools.push_back(p);
	}

	full_pools.clear();
}

void VulkanDescriptorAllocator::destroy_pools(VkDevice device) {
	for (auto p : ready_pools) {
		vkDestroyDescriptorPool(device, p, nullptr);
	}
	for (auto p : full_pools) {
		vkDestroyDescriptorPool(device, p, nullptr);
		ready_pools.push_back(p);
	}

	full_pools.clear();
}

VkDescriptorSet VulkanDescriptorAllocator::allocate(
		VkDevice device, VkDescriptorSetLayout layout, void* next) {
	VkDescriptorPool pool_to_use = get_pool(device);

	VkDescriptorSetAllocateInfo alloc_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.pNext = next,
		.descriptorPool = pool_to_use,
		.descriptorSetCount = 1,
		.pSetLayouts = &layout,
	};

	VkDescriptorSet ds;

	VkResult res = vkAllocateDescriptorSets(device, &alloc_info, &ds);
	if (res == VK_ERROR_OUT_OF_POOL_MEMORY || res == VK_ERROR_FRAGMENTED_POOL) {
		full_pools.push_back(pool_to_use);

		pool_to_use = get_pool(device);
		alloc_info.descriptorPool = pool_to_use;

		VK_CHECK(vkAllocateDescriptorSets(device, &alloc_info, &ds));
	}

	ready_pools.push_back(pool_to_use);

	return ds;
}

VkDescriptorPool VulkanDescriptorAllocator::get_pool(VkDevice device) {
	VkDescriptorPool new_pool;
	if (ready_pools.size() != 0) {
		new_pool = ready_pools.back();
		ready_pools.pop_back();
	} else {
		// need to create a new pool
		new_pool = create_pool(device, sets_per_pool, ratios);

		sets_per_pool *= 1.5f;
		if (sets_per_pool > 4092) {
			sets_per_pool = 4092;
		}
	}

	return new_pool;
}

VkDescriptorPool VulkanDescriptorAllocator::create_pool(VkDevice device,
		uint32_t set_count, std::span<PoolSizeRatio> pool_ratios) {
	std::vector<VkDescriptorPoolSize> pool_sizes;
	for (PoolSizeRatio ratio : pool_ratios) {
		pool_sizes.push_back(VkDescriptorPoolSize{
				.type = ratio.type,
				.descriptorCount =
						static_cast<uint32_t>(ratio.ratio * set_count),
		});
	}

	VkDescriptorPoolCreateInfo pool_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.flags = 0,
		.maxSets = set_count,
		.poolSizeCount = static_cast<uint32_t>(pool_sizes.size()),
		.pPoolSizes = pool_sizes.data(),
	};

	VkDescriptorPool new_pool;
	vkCreateDescriptorPool(device, &pool_info, nullptr, &new_pool);

	return new_pool;
}

void DescriptorWriter::write_buffer(int binding, VkBuffer buffer, size_t size,
		size_t offset, VkDescriptorType type) {
	VkDescriptorBufferInfo& info =
			buffer_infos.emplace_back(VkDescriptorBufferInfo{
					.buffer = buffer,
					.offset = offset,
					.range = size,
			});

	VkWriteDescriptorSet write = {
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = VK_NULL_HANDLE, //left empty for now until we need to write it
		.dstBinding = static_cast<uint32_t>(binding),
		.descriptorCount = 1,
		.descriptorType = type,
		.pBufferInfo = &info,
	};

	writes.push_back(write);
}

void DescriptorWriter::write_image(int binding, VkImageView image,
		VkSampler sampler, VkImageLayout layout, VkDescriptorType type) {
	VkDescriptorImageInfo& info =
			image_infos.emplace_back(VkDescriptorImageInfo{
					.sampler = sampler,
					.imageView = image,
					.imageLayout = layout,
			});

	VkWriteDescriptorSet write = {
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = VK_NULL_HANDLE, //left empty for now until we need to write it
		.dstBinding = static_cast<uint32_t>(binding),
		.descriptorCount = 1,
		.descriptorType = type,
		.pImageInfo = &info,
	};

	writes.push_back(write);
}

void DescriptorWriter::clear() {
	image_infos.clear();
	writes.clear();
	buffer_infos.clear();
}

void DescriptorWriter::update_set(VkDevice device, VkDescriptorSet set) {
	for (VkWriteDescriptorSet& write : writes) {
		write.dstSet = set;
	}

	vkUpdateDescriptorSets(
			device, (uint32_t)writes.size(), writes.data(), 0, nullptr);
}
