#include "platform/vulkan/vk_backend.h"

UniformSet VulkanRenderBackend::uniform_set_create(
		VectorView<ShaderUniform> p_uniforms, Shader p_shader,
		uint32_t p_set_index) {
	DescriptorSetPoolKey pool_key;

	std::vector<VkWriteDescriptorSet> vk_writes;

	std::unordered_map<size_t, VkDescriptorImageInfo> vk_image_infos;
	std::unordered_map<size_t, VkDescriptorBufferInfo> vk_buffer_infos;

	for (uint32_t i = 0; i < p_uniforms.size(); i++) {
		const ShaderUniform& uniform = p_uniforms[i];

		VkWriteDescriptorSet vk_write = {};
		vk_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		vk_write.dstBinding = uniform.binding;
		vk_write.descriptorType = VK_DESCRIPTOR_TYPE_MAX_ENUM;

		uint32_t num_descriptors = 1;

		switch (uniform.type) {
			case UNIFORM_TYPE_SAMPLER: {
				num_descriptors = uniform.data.size();

				vk_write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;

				VkDescriptorImageInfo vk_img_info = {};
				vk_img_info.sampler = (VkSampler)uniform.data[0];
				vk_img_info.imageView = VK_NULL_HANDLE;
				vk_img_info.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

				vk_image_infos[i] = std::move(vk_img_info);
			} break;
			case UNIFORM_TYPE_SAMPLER_WITH_TEXTURE: {
				num_descriptors = uniform.data.size() / 2;

				vk_write.descriptorType =
						VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

				VkDescriptorImageInfo vk_img_info = {};
				vk_img_info.sampler = (VkSampler)uniform.data[0];
				vk_img_info.imageView =
						((const VulkanImage*)uniform.data[1])->vk_image_view;
				vk_img_info.imageLayout =
						VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

				vk_image_infos[i] = std::move(vk_img_info);
			} break;
			case UNIFORM_TYPE_TEXTURE: {
				num_descriptors = uniform.data.size();

				vk_write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;

				VkDescriptorImageInfo vk_img_info = {};
				vk_img_info.imageView =
						((const VulkanImage*)uniform.data[0])->vk_image_view;
				vk_img_info.imageLayout =
						VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

				vk_image_infos[i] = std::move(vk_img_info);
			} break;
			case UNIFORM_TYPE_IMAGE: {
				num_descriptors = uniform.data.size();

				vk_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

				VkDescriptorImageInfo vk_img_info = {};
				vk_img_info.imageView =
						((VulkanImage*)uniform.data[0])->vk_image_view;
				vk_img_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

				vk_image_infos[i] = std::move(vk_img_info);
			} break;
			case UNIFORM_TYPE_UNIFORM_BUFFER: {
				const VulkanBuffer* buf_info =
						(const VulkanBuffer*)uniform.data[0];

				vk_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

				VkDescriptorBufferInfo vk_buf_info = {};
				vk_buf_info.buffer = buf_info->vk_buffer;
				vk_buf_info.range = buf_info->size;

				vk_buffer_infos[i] = std::move(vk_buf_info);
			} break;
			case UNIFORM_TYPE_STORAGE_BUFFER: {
				const VulkanBuffer* buf_info =
						(const VulkanBuffer*)uniform.data[0];

				vk_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

				VkDescriptorBufferInfo vk_buf_info = {};
				vk_buf_info.buffer = buf_info->vk_buffer;
				vk_buf_info.range = buf_info->size;

				vk_buffer_infos[i] = std::move(vk_buf_info);
			} break;
			default: {
				GL_ASSERT(false);
			}
		}

		vk_write.descriptorCount = num_descriptors;

		vk_writes.push_back(std::move(vk_write));

		if (pool_key.uniform_type[uniform.type] == MAX_UNIFORM_POOL_ELEMENT) {
			GL_LOG_ERROR("Uniform set reached the limit of bindings for the "
						 "same type ({})",
					static_cast<int>(MAX_UNIFORM_POOL_ELEMENT));

			return UniformSet();
		}
		pool_key.uniform_type[uniform.type] += num_descriptors;
	}

	// Need a descriptor pool.
	DescriptorSetPools::iterator pool_sets_it = {};
	VkDescriptorPool vk_pool = (VkDescriptorPool)_uniform_pool_find_or_create(
			pool_key, &pool_sets_it);
	GL_ASSERT(vk_pool);
	pool_sets_it->second[vk_pool]++;

	VkDescriptorSetAllocateInfo descriptor_set_allocate_info = {};
	descriptor_set_allocate_info.sType =
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptor_set_allocate_info.descriptorPool = vk_pool;
	descriptor_set_allocate_info.descriptorSetCount = 1;

	const VulkanShader* shader_info = (const VulkanShader*)p_shader;
	descriptor_set_allocate_info.pSetLayouts =
			&shader_info->descriptor_set_layouts[p_set_index];

	VkDescriptorSet vk_descriptor_set = VK_NULL_HANDLE;
	VkResult res = vkAllocateDescriptorSets(
			device, &descriptor_set_allocate_info, &vk_descriptor_set);
	if (res) {
		_uniform_pool_unreference(pool_sets_it, vk_pool);

		GL_LOG_ERROR("Cannot allocate descriptor sets, error {}.",
				static_cast<int>(res));

		return UniformSet();
	}

	for (const auto& img_info : vk_image_infos) {
		vk_writes[img_info.first].pImageInfo = &img_info.second;
	}

	for (const auto& buf_info : vk_buffer_infos) {
		vk_writes[buf_info.first].pBufferInfo = &buf_info.second;
	}

	for (uint32_t i = 0; i < p_uniforms.size(); i++) {
		vk_writes[i].dstSet = vk_descriptor_set;
	}

	vkUpdateDescriptorSets(
			device, p_uniforms.size(), vk_writes.data(), 0, nullptr);

	// Bookkeep.
	VulkanUniformSet* usi =
			VersatileResource::allocate<VulkanUniformSet>(resources_allocator);
	usi->vk_descriptor_set = vk_descriptor_set;
	usi->vk_descriptor_pool = vk_pool;
	usi->pool_sets_it = pool_sets_it;

	return UniformSet(usi);
}

void VulkanRenderBackend::uniform_set_free(UniformSet p_uniform_set) {
	if (!p_uniform_set) {
		return;
	}

	VulkanUniformSet* usi = (VulkanUniformSet*)p_uniform_set;

	vkFreeDescriptorSets(
			device, usi->vk_descriptor_pool, 1, &usi->vk_descriptor_set);

	_uniform_pool_unreference(usi->pool_sets_it, usi->vk_descriptor_pool);

	VersatileResource::free(resources_allocator, usi);
}

static const uint32_t MAX_DESCRIPTOR_SETS_PER_POOL = 10;

VkDescriptorPool VulkanRenderBackend::_uniform_pool_find_or_create(
		const DescriptorSetPoolKey& p_key,
		DescriptorSetPools::iterator* r_pool_sets_it) {
	DescriptorSetPools::iterator pool_sets_it =
			descriptor_set_pools.find(p_key);

	if (pool_sets_it != descriptor_set_pools.end()) {
		for (auto& pair : pool_sets_it->second) {
			if (pair.second < MAX_DESCRIPTOR_SETS_PER_POOL) {
				*r_pool_sets_it = pool_sets_it;
			}
		}
	}

	// Create a new one.
	std::vector<VkDescriptorPoolSize> vk_sizes;
	{
		VkDescriptorPoolSize curr_vk_size;
		const auto reset_vk_size = [&]() {
			memset(&curr_vk_size, 0, sizeof(VkDescriptorPoolSize));
		};

		if (p_key.uniform_type[UNIFORM_TYPE_SAMPLER]) {
			reset_vk_size();
			curr_vk_size.type = VK_DESCRIPTOR_TYPE_SAMPLER;
			curr_vk_size.descriptorCount =
					p_key.uniform_type[UNIFORM_TYPE_SAMPLER] *
					MAX_DESCRIPTOR_SETS_PER_POOL;

			vk_sizes.push_back(curr_vk_size);
		}
		if (p_key.uniform_type[UNIFORM_TYPE_SAMPLER_WITH_TEXTURE]) {
			reset_vk_size();
			curr_vk_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			curr_vk_size.descriptorCount =
					p_key.uniform_type[UNIFORM_TYPE_SAMPLER_WITH_TEXTURE] *
					MAX_DESCRIPTOR_SETS_PER_POOL;

			vk_sizes.push_back(curr_vk_size);
		}
		if (p_key.uniform_type[UNIFORM_TYPE_TEXTURE]) {
			reset_vk_size();
			curr_vk_size.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			curr_vk_size.descriptorCount =
					p_key.uniform_type[UNIFORM_TYPE_TEXTURE] *
					MAX_DESCRIPTOR_SETS_PER_POOL;

			vk_sizes.push_back(curr_vk_size);
		}
		if (p_key.uniform_type[UNIFORM_TYPE_IMAGE]) {
			reset_vk_size();
			curr_vk_size.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			curr_vk_size.descriptorCount =
					p_key.uniform_type[UNIFORM_TYPE_IMAGE] *
					MAX_DESCRIPTOR_SETS_PER_POOL;

			vk_sizes.push_back(curr_vk_size);
		}
		if (p_key.uniform_type[UNIFORM_TYPE_UNIFORM_BUFFER]) {
			reset_vk_size();
			curr_vk_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			curr_vk_size.descriptorCount =
					p_key.uniform_type[UNIFORM_TYPE_UNIFORM_BUFFER] *
					MAX_DESCRIPTOR_SETS_PER_POOL;

			vk_sizes.push_back(curr_vk_size);
		}
		if (p_key.uniform_type[UNIFORM_TYPE_STORAGE_BUFFER]) {
			reset_vk_size();
			curr_vk_size.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			curr_vk_size.descriptorCount =
					p_key.uniform_type[UNIFORM_TYPE_STORAGE_BUFFER] *
					MAX_DESCRIPTOR_SETS_PER_POOL;

			vk_sizes.push_back(curr_vk_size);
		}

		GL_ASSERT(vk_sizes.size() <= UNIFORM_TYPE_MAX);
	}

	VkDescriptorPoolCreateInfo descriptor_set_pool_create_info = {};
	descriptor_set_pool_create_info.sType =
			VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptor_set_pool_create_info.flags =
			VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

	descriptor_set_pool_create_info.maxSets = MAX_DESCRIPTOR_SETS_PER_POOL;
	descriptor_set_pool_create_info.poolSizeCount = (uint32_t)vk_sizes.size();
	descriptor_set_pool_create_info.pPoolSizes = vk_sizes.data();

	VkDescriptorPool vk_pool = VK_NULL_HANDLE;
	VK_CHECK(vkCreateDescriptorPool(
			device, &descriptor_set_pool_create_info, nullptr, &vk_pool));

	// Bookkeep.
	if (pool_sets_it == descriptor_set_pools.end()) {
		pool_sets_it = descriptor_set_pools
							   .emplace(p_key,
									   std::unordered_map<VkDescriptorPool,
											   uint32_t>())
							   .first;
	}

	std::unordered_map<VkDescriptorPool, uint32_t>& pool_rcs =
			pool_sets_it->second;
	pool_rcs.emplace(vk_pool, 0);
	*r_pool_sets_it = pool_sets_it;

	return vk_pool;
}

void VulkanRenderBackend::_uniform_pool_unreference(
		DescriptorSetPools::iterator p_pool_sets_it,
		VkDescriptorPool p_vk_descriptor_pool) {
	std::unordered_map<VkDescriptorPool, uint32_t>::iterator pool_rcs_it =
			p_pool_sets_it->second.find(p_vk_descriptor_pool);
	pool_rcs_it->second--;
	if (pool_rcs_it->second == 0) {
		vkDestroyDescriptorPool(device, p_vk_descriptor_pool, nullptr);
		p_pool_sets_it->second.erase(p_vk_descriptor_pool);
		if (p_pool_sets_it->second.empty()) {
			descriptor_set_pools.erase(p_pool_sets_it);
		}
	}
}
