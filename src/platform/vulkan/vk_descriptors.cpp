#include "platform/vulkan/vk_descriptors.h"

#include "platform/vulkan/vk_context.h"
#include "platform/vulkan/vk_image.h"

namespace vk {

static const uint32_t MAX_DESCRIPTOR_SETS_PER_POOL = 10;

UniformPool uniform_pool_find_or_create(Context p_context,
		const DescriptorSetPoolKey& p_key,
		DescriptorSetPools::iterator* r_pool_sets_it) {
	VulkanContext* context = (VulkanContext*)p_context;

	DescriptorSetPools::iterator pool_sets_it =
			context->descriptor_set_pools.find(p_key);

	if (pool_sets_it != context->descriptor_set_pools.end()) {
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
		if (p_key.uniform_type[UNIFORM_TYPE_INPUT_ATTACHMENT]) {
			reset_vk_size();
			curr_vk_size.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
			curr_vk_size.descriptorCount =
					p_key.uniform_type[UNIFORM_TYPE_INPUT_ATTACHMENT] *
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
	VK_CHECK(vkCreateDescriptorPool(context->device,
			&descriptor_set_pool_create_info, nullptr, &vk_pool));

	// Bookkeep.
	if (pool_sets_it == context->descriptor_set_pools.end()) {
		pool_sets_it = context->descriptor_set_pools
							   .emplace(p_key,
									   std::unordered_map<VkDescriptorPool,
											   uint32_t>())
							   .first;
	}

	std::unordered_map<VkDescriptorPool, uint32_t>& pool_rcs =
			pool_sets_it->second;
	pool_rcs.emplace(vk_pool, 0);
	*r_pool_sets_it = pool_sets_it;

	return UniformPool(vk_pool);
}

void uniform_pool_unreference(Context p_context,
		DescriptorSetPools::iterator p_pool_sets_it,
		VkDescriptorPool p_vk_descriptor_pool) {
	VulkanContext* context = (VulkanContext*)p_context;

	std::unordered_map<VkDescriptorPool, uint32_t>::iterator pool_rcs_it =
			p_pool_sets_it->second.find(p_vk_descriptor_pool);
	pool_rcs_it->second--;
	if (pool_rcs_it->second == 0) {
		vkDestroyDescriptorPool(context->device, p_vk_descriptor_pool, nullptr);
		p_pool_sets_it->second.erase(p_vk_descriptor_pool);
		if (p_pool_sets_it->second.empty()) {
			context->descriptor_set_pools.erase(p_pool_sets_it);
		}
	}
}

UniformSet uniform_set_create(Context p_context,
		VectorView<BoundUniform> p_uniforms, Shader p_shader,
		uint32_t p_set_index) {
	DescriptorSetPoolKey pool_key;

	std::vector<VkWriteDescriptorSet> vk_writes;
	for (uint32_t i = 0; i < p_uniforms.size(); i++) {
		const BoundUniform& uniform = p_uniforms[i];

		VkWriteDescriptorSet vk_write = {};
		vk_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		vk_write.dstBinding = uniform.binding;
		vk_write.descriptorType = VK_DESCRIPTOR_TYPE_MAX_ENUM; // Invalid value.

		uint32_t num_descriptors = 1;

		switch (uniform.type) {
			case UNIFORM_TYPE_SAMPLER: {
				num_descriptors = uniform.ids.size();

				VkDescriptorImageInfo vk_img_info = {};
				vk_img_info.sampler = (VkSampler)uniform.ids[0];
				vk_img_info.imageView = VK_NULL_HANDLE;
				vk_img_info.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

				vk_write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
				vk_write.pImageInfo = &vk_img_info;
			} break;
			case UNIFORM_TYPE_SAMPLER_WITH_TEXTURE: {
				num_descriptors = uniform.ids.size() / 2;

				VkDescriptorImageInfo vk_img_info = {};
				vk_img_info.sampler = (VkSampler)uniform.ids[0];
				vk_img_info.imageView =
						((const VulkanImage*)uniform.ids[1])->vk_image_view;
				vk_img_info.imageLayout =
						VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

				vk_write.descriptorType =
						VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				vk_write.pImageInfo = &vk_img_info;

			} break;
			case UNIFORM_TYPE_TEXTURE: {
				num_descriptors = uniform.ids.size();

				VkDescriptorImageInfo vk_img_info = {};
				vk_img_info.imageView =
						((const VulkanImage*)uniform.ids[0])->vk_image_view;
				vk_img_info.imageLayout =
						VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

				vk_write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
				vk_write.pImageInfo = &vk_img_info;

			} break;
			case UNIFORM_TYPE_IMAGE: {
				num_descriptors = uniform.ids.size();

				VkDescriptorImageInfo vk_img_info = {};
				vk_img_info.imageView =
						((VulkanImage*)uniform.ids[0])->vk_image_view;
				vk_img_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

				vk_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
				vk_write.pImageInfo = &vk_img_info;

			} break;
			case UNIFORM_TYPE_UNIFORM_BUFFER: {
				const VulkanBuffer* buf_info =
						(const VulkanBuffer*)uniform.ids[0];

				VkDescriptorBufferInfo vk_buf_info = {};
				vk_buf_info.buffer = buf_info->vk_buffer;
				vk_buf_info.range = buf_info->size;

				vk_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				vk_write.pBufferInfo = &vk_buf_info;

			} break;
			case UNIFORM_TYPE_STORAGE_BUFFER: {
				const VulkanBuffer* buf_info =
						(const VulkanBuffer*)uniform.ids[0];

				VkDescriptorBufferInfo vk_buf_info = {};
				vk_buf_info.buffer = buf_info->vk_buffer;
				vk_buf_info.range = buf_info->size;

				vk_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				vk_write.pBufferInfo = &vk_buf_info;

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

					vk_img_infos.push_back(vk_img_info);
				}

				vk_write.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
				vk_write.pImageInfo = vk_img_infos.data();

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

	VulkanContext* context = (VulkanContext*)p_context;

	// Need a descriptor pool.
	DescriptorSetPools::iterator pool_sets_it = {};
	VkDescriptorPool vk_pool = (VkDescriptorPool)uniform_pool_find_or_create(
			p_context, pool_key, &pool_sets_it);
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
			context->device, &descriptor_set_allocate_info, &vk_descriptor_set);
	if (res) {
		uniform_pool_unreference(p_context, pool_sets_it, vk_pool);

		GL_LOG_ERROR("Cannot allocate descriptor sets, error {}.",
				static_cast<int>(res));

		return UniformSet();
	}

	for (uint32_t i = 0; i < p_uniforms.size(); i++) {
		vk_writes[i].dstSet = vk_descriptor_set;
	}

	vkUpdateDescriptorSets(
			context->device, p_uniforms.size(), vk_writes.data(), 0, nullptr);

	// Bookkeep.
	VulkanUniformSet* usi = VersatileResource::allocate<VulkanUniformSet>(
			context->resources_allocator);
	usi->vk_descriptor_set = vk_descriptor_set;
	usi->vk_descriptor_pool = vk_pool;
	usi->pool_sets_it = pool_sets_it;

	return UniformSet(usi);
}

void uniform_set_free(Context p_context, UniformSet p_uniform_set) {
	if (!p_uniform_set) {
		return;
	}

	VulkanContext* context = (VulkanContext*)p_context;
	VulkanUniformSet* usi = (VulkanUniformSet*)p_uniform_set;

	vkFreeDescriptorSets(context->device, usi->vk_descriptor_pool, 1,
			&usi->vk_descriptor_set);

	uniform_pool_unreference(
			p_context, usi->pool_sets_it, usi->vk_descriptor_pool);

	VersatileResource::free(context->resources_allocator, usi);
}

} //namespace vk
