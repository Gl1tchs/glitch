#include "backends/vulkan/vk_descriptors.h"

#include "backends/vulkan/vk_context.h"
#include "backends/vulkan/vk_image.h"

namespace vk {

VkDescriptorPool descriptor_set_pool_find_or_create(Context p_context,
		const DescriptorSetPoolKey& p_key,
		DescriptorSetPools::iterator* r_pool_sets_it) {
	VulkanContext* context = (VulkanContext*)p_context;

	DescriptorSetPools::iterator pool_sets_it =
			context->descriptor_set_pools.find(p_key);

	if (pool_sets_it != context->descriptor_set_pools.end()) {
		for (auto& pair : pool_sets_it->second) {
			if (pair.second < context->max_descriptor_sets_per_pool) {
				*r_pool_sets_it = pool_sets_it;
			}
		}
	}

	// Create a new one.
	std::vector<VkDescriptorPoolSize> vk_sizes(UNIFORM_TYPE_MAX);
	uint32_t vk_sizes_count = 0;
	{
		VkDescriptorPoolSize* curr_vk_size = vk_sizes.data();
		if (p_key.uniform_type[UNIFORM_TYPE_SAMPLER]) {
			*curr_vk_size = {};
			curr_vk_size->type = VK_DESCRIPTOR_TYPE_SAMPLER;
			curr_vk_size->descriptorCount =
					p_key.uniform_type[UNIFORM_TYPE_SAMPLER] *
					context->max_descriptor_sets_per_pool;
			curr_vk_size++;
			vk_sizes_count++;
		}
		if (p_key.uniform_type[UNIFORM_TYPE_SAMPLER_WITH_TEXTURE]) {
			*curr_vk_size = {};
			curr_vk_size->type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			curr_vk_size->descriptorCount =
					p_key.uniform_type[UNIFORM_TYPE_SAMPLER_WITH_TEXTURE] *
					context->max_descriptor_sets_per_pool;
			curr_vk_size++;
			vk_sizes_count++;
		}
		if (p_key.uniform_type[UNIFORM_TYPE_TEXTURE]) {
			*curr_vk_size = {};
			curr_vk_size->type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			curr_vk_size->descriptorCount =
					p_key.uniform_type[UNIFORM_TYPE_TEXTURE] *
					context->max_descriptor_sets_per_pool;
			curr_vk_size++;
			vk_sizes_count++;
		}
		if (p_key.uniform_type[UNIFORM_TYPE_IMAGE]) {
			*curr_vk_size = {};
			curr_vk_size->type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			curr_vk_size->descriptorCount =
					p_key.uniform_type[UNIFORM_TYPE_IMAGE] *
					context->max_descriptor_sets_per_pool;
			curr_vk_size++;
			vk_sizes_count++;
		}
		if (p_key.uniform_type[UNIFORM_TYPE_UNIFORM_BUFFER]) {
			*curr_vk_size = {};
			curr_vk_size->type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			curr_vk_size->descriptorCount =
					p_key.uniform_type[UNIFORM_TYPE_UNIFORM_BUFFER] *
					context->max_descriptor_sets_per_pool;
			curr_vk_size++;
			vk_sizes_count++;
		}
		if (p_key.uniform_type[UNIFORM_TYPE_STORAGE_BUFFER]) {
			*curr_vk_size = {};
			curr_vk_size->type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			curr_vk_size->descriptorCount =
					p_key.uniform_type[UNIFORM_TYPE_STORAGE_BUFFER] *
					context->max_descriptor_sets_per_pool;
			curr_vk_size++;
			vk_sizes_count++;
		}
		if (p_key.uniform_type[UNIFORM_TYPE_INPUT_ATTACHMENT]) {
			*curr_vk_size = {};
			curr_vk_size->type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
			curr_vk_size->descriptorCount =
					p_key.uniform_type[UNIFORM_TYPE_INPUT_ATTACHMENT] *
					context->max_descriptor_sets_per_pool;
			curr_vk_size++;
			vk_sizes_count++;
		}
		GL_ASSERT(vk_sizes_count <= UNIFORM_TYPE_MAX);
	}

	VkDescriptorPoolCreateInfo descriptor_set_pool_create_info = {};
	descriptor_set_pool_create_info.sType =
			VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptor_set_pool_create_info.flags =
			VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

	descriptor_set_pool_create_info.maxSets =
			context->max_descriptor_sets_per_pool;
	descriptor_set_pool_create_info.poolSizeCount = vk_sizes_count;
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

	return vk_pool;
}

void descriptor_set_pool_unreference(Context p_context,
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

	std::vector<VkWriteDescriptorSet> vk_writes(p_uniforms.size());
	for (uint32_t i = 0; i < p_uniforms.size(); i++) {
		const BoundUniform& uniform = p_uniforms[i];

		vk_writes[i] = {};
		vk_writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		vk_writes[i].dstBinding = uniform.binding;
		vk_writes[i].descriptorType =
				VK_DESCRIPTOR_TYPE_MAX_ENUM; // Invalid value.

		uint32_t num_descriptors = 1;

		switch (uniform.type) {
			case UNIFORM_TYPE_SAMPLER: {
				num_descriptors = uniform.ids.size();
				std::vector<VkDescriptorImageInfo> vk_img_infos(
						num_descriptors);

				for (uint32_t j = 0; j < num_descriptors; j++) {
					vk_img_infos[j] = {};
					vk_img_infos[j].sampler = (VkSampler)uniform.ids[j];
					vk_img_infos[j].imageView = VK_NULL_HANDLE;
					vk_img_infos[j].imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				}

				vk_writes[i].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
				vk_writes[i].pImageInfo = vk_img_infos.data();
			} break;
			case UNIFORM_TYPE_SAMPLER_WITH_TEXTURE: {
				num_descriptors = uniform.ids.size() / 2;
				std::vector<VkDescriptorImageInfo> vk_img_infos(
						num_descriptors);

				for (uint32_t j = 0; j < num_descriptors; j++) {
					vk_img_infos[j] = {};
					vk_img_infos[j].sampler = (VkSampler)uniform.ids[j * 2 + 0];
					vk_img_infos[j].imageView =
							((const VulkanImage*)uniform.ids[j * 2 + 1])
									->vk_image_view;
					vk_img_infos[j].imageLayout =
							VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				}

				vk_writes[i].descriptorType =
						VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				vk_writes[i].pImageInfo = vk_img_infos.data();
			} break;
			case UNIFORM_TYPE_TEXTURE: {
				num_descriptors = uniform.ids.size();
				std::vector<VkDescriptorImageInfo> vk_img_infos(
						num_descriptors);

				for (uint32_t j = 0; j < num_descriptors; j++) {
					vk_img_infos[j] = {};
					vk_img_infos[j].imageView =
							((const VulkanImage*)uniform.ids[j])->vk_image_view;
					vk_img_infos[j].imageLayout =
							VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				}

				vk_writes[i].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
				vk_writes[i].pImageInfo = vk_img_infos.data();
			} break;
			case UNIFORM_TYPE_IMAGE: {
				num_descriptors = uniform.ids.size();
				std::vector<VkDescriptorImageInfo> vk_img_infos(
						num_descriptors);

				for (uint32_t j = 0; j < num_descriptors; j++) {
					vk_img_infos[j] = {};
					vk_img_infos[j].imageView =
							((const VulkanImage*)uniform.ids[j])->vk_image_view;
					vk_img_infos[j].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
				}

				vk_writes[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
				vk_writes[i].pImageInfo = vk_img_infos.data();
			} break;
			case UNIFORM_TYPE_UNIFORM_BUFFER: {
				const VulkanBuffer* buf_info =
						(const VulkanBuffer*)uniform.ids[0];
				VkDescriptorBufferInfo vk_buf_info = {};
				vk_buf_info.buffer = buf_info->vk_buffer;
				vk_buf_info.range = buf_info->size;

				vk_writes[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				vk_writes[i].pBufferInfo = &vk_buf_info;
			} break;
			case UNIFORM_TYPE_STORAGE_BUFFER: {
				const VulkanBuffer* buf_info =
						(const VulkanBuffer*)uniform.ids[0];
				VkDescriptorBufferInfo vk_buf_info = {};
				vk_buf_info.buffer = buf_info->vk_buffer;
				vk_buf_info.range = buf_info->size;

				vk_writes[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				vk_writes[i].pBufferInfo = &vk_buf_info;
			} break;
			case UNIFORM_TYPE_INPUT_ATTACHMENT: {
				num_descriptors = uniform.ids.size();
				std::vector<VkDescriptorImageInfo> vk_img_infos(
						num_descriptors);

				for (uint32_t j = 0; j < uniform.ids.size(); j++) {
					vk_img_infos[j] = {};
					vk_img_infos[j].imageView =
							((const VulkanImage*)uniform.ids[j])->vk_image_view;
					vk_img_infos[j].imageLayout =
							VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				}

				vk_writes[i].descriptorType =
						VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
				vk_writes[i].pImageInfo = vk_img_infos.data();
			} break;
			default: {
				GL_ASSERT(false);
			}
		}

		vk_writes[i].descriptorCount = num_descriptors;

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
	VkDescriptorPool vk_pool = descriptor_set_pool_find_or_create(
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
		descriptor_set_pool_unreference(p_context, pool_sets_it, vk_pool);

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
	VulkanContext* context = (VulkanContext*)p_context;
	VulkanUniformSet* usi = (VulkanUniformSet*)p_uniform_set;

	vkFreeDescriptorSets(context->device, usi->vk_descriptor_pool, 1,
			&usi->vk_descriptor_set);

	descriptor_set_pool_unreference(
			p_context, usi->pool_sets_it, usi->vk_descriptor_pool);

	VersatileResource::free(context->resources_allocator, usi);
}

} //namespace vk