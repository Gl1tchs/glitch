#include "platform/vulkan/vk_backend.h"

UniformSet VulkanRenderBackend::uniform_set_create(
		VectorView<BoundUniform> p_uniforms, Shader p_shader,
		uint32_t p_set_index) {
	DescriptorSetPoolKey pool_key;

	std::vector<VkWriteDescriptorSet> vk_writes;
	std::vector<VkDescriptorImageInfo> vk_image_infos;
	std::vector<VkDescriptorBufferInfo> vk_buffer_infos;

	for (uint32_t i = 0; i < p_uniforms.size(); i++) {
		const BoundUniform& uniform = p_uniforms[i];

		VkWriteDescriptorSet vk_write = {};
		vk_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		vk_write.dstBinding = uniform.binding;
		vk_write.descriptorType = VK_DESCRIPTOR_TYPE_MAX_ENUM;

		uint32_t num_descriptors = 1;

		switch (uniform.type) {
			case UNIFORM_TYPE_SAMPLER: {
				num_descriptors = uniform.ids.size();

				VkDescriptorImageInfo vk_img_info = {};
				vk_img_info.sampler = (VkSampler)uniform.ids[0];
				vk_img_info.imageView = VK_NULL_HANDLE;
				vk_img_info.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

				vk_image_infos.push_back(vk_img_info);
				vk_write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
				vk_write.pImageInfo = &vk_image_infos.back();
			} break;
			case UNIFORM_TYPE_SAMPLER_WITH_TEXTURE: {
				num_descriptors = uniform.ids.size() / 2;

				VkDescriptorImageInfo vk_img_info = {};
				vk_img_info.sampler = (VkSampler)uniform.ids[0];
				vk_img_info.imageView =
						((const VulkanImage*)uniform.ids[1])->vk_image_view;
				vk_img_info.imageLayout =
						VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

				vk_image_infos.push_back(vk_img_info);
				vk_write.descriptorType =
						VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				vk_write.pImageInfo = &vk_image_infos.back();
			} break;
			case UNIFORM_TYPE_TEXTURE: {
				num_descriptors = uniform.ids.size();

				VkDescriptorImageInfo vk_img_info = {};
				vk_img_info.imageView =
						((const VulkanImage*)uniform.ids[0])->vk_image_view;
				vk_img_info.imageLayout =
						VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

				vk_image_infos.push_back(vk_img_info);
				vk_write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
				vk_write.pImageInfo = &vk_image_infos.back();
			} break;
			case UNIFORM_TYPE_IMAGE: {
				num_descriptors = uniform.ids.size();

				VkDescriptorImageInfo vk_img_info = {};
				vk_img_info.imageView =
						((VulkanImage*)uniform.ids[0])->vk_image_view;
				vk_img_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

				vk_image_infos.push_back(vk_img_info);
				vk_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
				vk_write.pImageInfo = &vk_image_infos.back();
			} break;
			case UNIFORM_TYPE_UNIFORM_BUFFER: {
				const VulkanBuffer* buf_info =
						(const VulkanBuffer*)uniform.ids[0];

				VkDescriptorBufferInfo vk_buf_info = {};
				vk_buf_info.buffer = buf_info->vk_buffer;
				vk_buf_info.range = buf_info->size;

				vk_buffer_infos.push_back(vk_buf_info);
				vk_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				vk_write.pBufferInfo = &vk_buffer_infos.back();
			} break;
			case UNIFORM_TYPE_STORAGE_BUFFER: {
				const VulkanBuffer* buf_info =
						(const VulkanBuffer*)uniform.ids[0];

				VkDescriptorBufferInfo vk_buf_info = {};
				vk_buf_info.buffer = buf_info->vk_buffer;
				vk_buf_info.range = buf_info->size;

				vk_buffer_infos.push_back(vk_buf_info);
				vk_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				vk_write.pBufferInfo = &vk_buffer_infos.back();
			} break;
			case UNIFORM_TYPE_INPUT_ATTACHMENT: {
				num_descriptors = uniform.ids.size();
				std::vector<VkDescriptorImageInfo> vk_img_infos;

				for (uint32_t j = 0; j < uniform.ids.size(); j++) {
					VkDescriptorImageInfo vk_img_info = {};
					vk_img_info.imageView =
							((const VulkanImage*)uniform.ids[j])->vk_image_view;
					vk_img_info.imageLayout =
							VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

					vk_image_infos.push_back(vk_img_info);
				}

				vk_write.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
				vk_write.pImageInfo = vk_image_infos.data();
			} break;
			default: {
				GL_ASSERT(false);
			}
		}

		vk_write.descriptorCount = num_descriptors;

		vk_writes.push_back(vk_write);

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
