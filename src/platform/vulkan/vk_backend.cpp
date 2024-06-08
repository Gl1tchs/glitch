#include "platform/vulkan/vk_backend.h"

#include "platform/vulkan/vk_common.h"
#include "renderer/types.h"

#include <VkBootstrap.h>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

inline static VKAPI_ATTR VkBool32 VKAPI_CALL _vk_debug_callback(
		VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
		VkDebugUtilsMessageTypeFlagsEXT message_type,
		const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
		void* user_data) {
	if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
		GL_LOG_ERROR("{}", callback_data->pMessage);
	} else if (message_severity &
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
		GL_LOG_WARNING("{}", callback_data->pMessage);
	} else if (message_severity &
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
		GL_LOG_INFO("{}", callback_data->pMessage);
	} else {
		GL_LOG_TRACE("{}", callback_data->pMessage);
	}

	return VK_FALSE;
}

VulkanRenderBackend* VulkanRenderBackend::s_instance = nullptr;

void VulkanRenderBackend::init(Ref<Window> window) {
	GL_ASSERT(s_instance == nullptr, "Only one backend can exist at a time.");
	s_instance = this;

	vkb::InstanceBuilder vkb_builder;
	auto vkb_instance_result = vkb_builder
									   .set_app_name("glitch")
#ifdef GL_DEBUG_BUILD
									   .enable_validation_layers()
									   .set_debug_callback(_vk_debug_callback)
#endif
									   .require_api_version(1, 3, 0)
									   .build();

	if (!vkb_instance_result.has_value()) {
		GL_ASSERT(false, "Unable to create Vulkan instance of version 1.3.0!");
		return;
	}

	vkb::Instance vkb_instance = vkb_instance_result.value();

	instance = vkb_instance.instance;
	debug_messenger = vkb_instance.debug_messenger;

	glfwCreateWindowSurface(instance, (GLFWwindow*)window->get_native_window(),
			nullptr, &surface);

	VkPhysicalDeviceVulkan13Features features13 = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
		.synchronization2 = true,
		.dynamicRendering = true,
	};

	VkPhysicalDeviceVulkan12Features features12 = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
		.descriptorIndexing = true,
		.bufferDeviceAddress = true,
	};

	vkb::PhysicalDeviceSelector vkb_device_selector{ vkb_instance };
	vkb::PhysicalDevice vkb_physical_device =
			vkb_device_selector.set_minimum_version(1, 3)
					.set_required_features_13(features13)
					.set_required_features_12(features12)
					.set_surface(surface)
					.select()
					.value();

	// create the final vulkan device
	vkb::DeviceBuilder vkb_device_builder{ vkb_physical_device };
	vkb::Device vkb_device = vkb_device_builder.build().value();

	physical_device = vkb_device.physical_device;

	physical_device_properties = vkb_device.physical_device.properties;
	physical_device_features = vkb_device.physical_device.features;

	device = vkb_device.device;

	graphics_queue = VulkanQueue{
		.queue = vkb_device.get_queue(vkb::QueueType::graphics).value(),
		.queue_family =
				vkb_device.get_queue_index(vkb::QueueType::graphics).value(),
	};

	present_queue = VulkanQueue{
		.queue = vkb_device.get_queue(vkb::QueueType::present).value(),
		.queue_family =
				vkb_device.get_queue_index(vkb::QueueType::present).value(),
	};

	present_queue = VulkanQueue{
		.queue = vkb_device.get_queue(vkb::QueueType::transfer).value(),
		.queue_family =
				vkb_device.get_queue_index(vkb::QueueType::transfer).value(),
	};

	deletion_queue.push_function([this]() {
		vkDestroySurfaceKHR(instance, surface, nullptr);
		vkDestroyDevice(device, nullptr);

		vkb::destroy_debug_utils_messenger(instance, debug_messenger, nullptr);
		vkDestroyInstance(instance, nullptr);
	});

	VmaAllocatorCreateInfo allocator_info = {};
	allocator_info.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
	allocator_info.physicalDevice = vkb_physical_device;
	allocator_info.device = device;
	allocator_info.instance = instance;

	vmaCreateAllocator(&allocator_info, &allocator);

	deletion_queue.push_function([this]() {
		while (small_allocs_pools.size()) {
			std::unordered_map<uint32_t, VmaPool>::iterator e =
					small_allocs_pools.begin();
			vmaDestroyPool(allocator, e->second);
			small_allocs_pools.erase(e);
		}
		vmaDestroyAllocator(allocator);
	});

#ifndef GL_DIST_BUILD
	// inform user about the chosen device
	GL_LOG_INFO("Vulkan Initialized:");
	GL_LOG_INFO("Device: {}", physical_device_properties.deviceName);
	GL_LOG_INFO("API: {}.{}.{}",
			VK_VERSION_MAJOR(physical_device_properties.apiVersion),
			VK_VERSION_MINOR(physical_device_properties.apiVersion),
			VK_VERSION_PATCH(physical_device_properties.apiVersion));
#endif

	imm_fence = fence_create();

	imm_command_pool = command_pool_create((CommandQueue)&command_queue);

	imm_command_buffer = command_pool_allocate(imm_command_pool);

	deletion_queue.push_function([this]() {
		command_pool_free(imm_command_pool);
		fence_free(imm_fence);
	});

	deletion_queue.push_function([this]() {
		for (const auto& pools : descriptor_set_pools) {
			for (const auto& pool : pools.second) {
				vkDestroyDescriptorPool(device, pool.first, nullptr);
			}
		}
	});
}

void VulkanRenderBackend::shutdown() { deletion_queue.flush(); }

void VulkanRenderBackend::device_wait() { vkDeviceWaitIdle(device); }

CommandQueue VulkanRenderBackend::device_get_queue(QueueType p_type) {
	return CommandQueue(
			p_type == QUEUE_TYPE_GRAPHICS ? &graphics_queue : &present_queue);
}

VmaPool VulkanRenderBackend::_find_or_create_small_allocs_pool(
		uint32_t p_mem_type_index) {
	if (small_allocs_pools.find(p_mem_type_index) != small_allocs_pools.end()) {
		return small_allocs_pools[p_mem_type_index];
	}

	VmaPoolCreateInfo pci = {};
	pci.memoryTypeIndex = p_mem_type_index;
	pci.flags = 0;
	pci.blockSize = 0;
	pci.minBlockCount = 0;
	pci.maxBlockCount = SIZE_MAX;
	pci.priority = 0.5f;
	pci.minAllocationAlignment = 0;
	pci.pMemoryAllocateNext = nullptr;

	VmaPool pool = VK_NULL_HANDLE;
	VK_CHECK(vmaCreatePool(allocator, &pci, &pool));

	small_allocs_pools[p_mem_type_index] =
			pool; // Don't try to create it again if failed the first time.

	return pool;
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
