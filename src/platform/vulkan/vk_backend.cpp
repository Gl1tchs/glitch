#include "platform/vulkan/vk_backend.h"

#include "platform/vulkan/vk_commands.h"
#include "platform/vulkan/vk_context.h"
#include "platform/vulkan/vk_sync.h"
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

VulkanContext* VulkanRenderBackend::s_context = nullptr;

Context VulkanRenderBackend::init(Ref<Window> window) {
	GL_ASSERT(s_context == nullptr, "Only one context can exist at a time.");
	s_context = new VulkanContext();

	vkb::InstanceBuilder builder;
	auto instance_result = builder.set_app_name("glitch")
#ifdef GL_DEBUG_BUILD
								   .enable_validation_layers()
								   .set_debug_callback(_vk_debug_callback)
#endif
								   .require_api_version(1, 3, 0)
								   .build();

	if (!instance_result.has_value()) {
		return nullptr;
	}

	vkb::Instance vkb_instance = instance_result.value();

	s_context->instance = vkb_instance.instance;
	s_context->debug_messenger = vkb_instance.debug_messenger;

	glfwCreateWindowSurface(s_context->instance,
			(GLFWwindow*)window->get_native_window(), nullptr,
			&s_context->surface);

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

	vkb::PhysicalDeviceSelector selector{ vkb_instance };
	vkb::PhysicalDevice physical_device =
			selector.set_minimum_version(1, 3)
					.set_required_features_13(features13)
					.set_required_features_12(features12)
					.set_surface(s_context->surface)
					.select()
					.value();

	// create the final vulkan device
	vkb::DeviceBuilder device_builder{ physical_device };
	vkb::Device vkb_device = device_builder.build().value();

	s_context->physical_device = vkb_device.physical_device;

	s_context->physical_device_properties =
			vkb_device.physical_device.properties;
	s_context->physical_device_features = vkb_device.physical_device.features;

	s_context->device = vkb_device.device;

	s_context->graphics_queue = VulkanQueue{
		.queue = vkb_device.get_queue(vkb::QueueType::graphics).value(),
		.queue_family =
				vkb_device.get_queue_index(vkb::QueueType::graphics).value(),
	};

	s_context->present_queue = VulkanQueue{
		.queue = vkb_device.get_queue(vkb::QueueType::present).value(),
		.queue_family =
				vkb_device.get_queue_index(vkb::QueueType::present).value(),
	};

	s_context->present_queue = VulkanQueue{
		.queue = vkb_device.get_queue(vkb::QueueType::transfer).value(),
		.queue_family =
				vkb_device.get_queue_index(vkb::QueueType::transfer).value(),
	};

	deletion_queue.push_function([this]() {
		vkDestroySurfaceKHR(s_context->instance, s_context->surface, nullptr);
		vkDestroyDevice(s_context->device, nullptr);

		vkb::destroy_debug_utils_messenger(
				s_context->instance, s_context->debug_messenger, nullptr);
		vkDestroyInstance(s_context->instance, nullptr);
	});

	VmaAllocatorCreateInfo allocator_info = {
		.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
		.physicalDevice = physical_device,
		.device = s_context->device,
		.instance = s_context->instance,
	};
	vmaCreateAllocator(&allocator_info, &s_context->allocator);

	deletion_queue.push_function([this]() {
		while (s_context->small_allocs_pools.size()) {
			std::unordered_map<uint32_t, VmaPool>::iterator e =
					s_context->small_allocs_pools.begin();
			vmaDestroyPool(s_context->allocator, e->second);
			s_context->small_allocs_pools.erase(e);
		}
		vmaDestroyAllocator(s_context->allocator);
	});

	// get information about the device
	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(physical_device, &properties);

	GL_LOG_INFO("Vulkan Initialized:");
	GL_LOG_INFO("Device: {}", properties.deviceName);
	GL_LOG_INFO("API: {}.{}.{}", VK_VERSION_MAJOR(properties.apiVersion),
			VK_VERSION_MINOR(properties.apiVersion),
			VK_VERSION_PATCH(properties.apiVersion));

	s_context->imm_fence = vk::fence_create((Context)s_context);

	s_context->imm_command_pool = vk::command_pool_create(
			(Context)s_context, (CommandQueue)&s_context->command_queue);

	s_context->imm_command_buffer = vk::command_pool_allocate(
			(Context)s_context, s_context->imm_command_pool);

	deletion_queue.push_function([=]() {
		vk::command_pool_free((Context)s_context, s_context->imm_command_pool);
		vk::fence_free((Context)s_context, s_context->imm_fence);
	});

	deletion_queue.push_function([=]() {
		for (const auto& pools : s_context->descriptor_set_pools) {
			for (const auto& pool : pools.second) {
				vkDestroyDescriptorPool(s_context->device, pool.first, nullptr);
			}
		}
	});

	return Context(s_context);
}

void VulkanRenderBackend::shutdown(Context device) {
	deletion_queue.flush();
	delete s_context;
}

void VulkanRenderBackend::wait_for_device() {
	VulkanContext* context = (VulkanContext*)s_context;
	vkDeviceWaitIdle(context->device);
}

CommandQueue VulkanRenderBackend::get_command_queue(QueueType p_type) {
	return CommandQueue(p_type == QUEUE_TYPE_GRAPHICS
					? &s_context->graphics_queue
					: &s_context->present_queue);
}
