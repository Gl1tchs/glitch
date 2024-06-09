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
