#include "backends/vulkan/vk_backend.h"

#include "backends/vulkan/vk_context.h"

#include <VkBootstrap.h>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

inline static VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
		VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
		VkDebugUtilsMessageTypeFlagsEXT message_type,
		const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
		void* user_data) {
	const auto ms = vkb::to_string_message_severity(message_severity);
	const auto mt = vkb::to_string_message_type(message_type);
	GL_LOG_TRACE("[{}: {}]\n{}", ms, mt, callback_data->pMessage);

	return VK_FALSE;
}

VulkanContext* VulkanRenderBackend::s_context = nullptr;

Context VulkanRenderBackend::init(Ref<Window> window) {
	GL_ASSERT(s_context == nullptr, "Only one context can exist at a time.");
	s_context = new VulkanContext();

	vkb::InstanceBuilder builder;
	auto inst_ret =
			builder.set_app_name("glitch")
					// TODO do not activate debug messenger on DIST build
					.enable_validation_layers()
					.set_debug_callback(vk_debug_callback)
					.require_api_version(1, 3, 0)
					.build();

	if (!inst_ret.has_value()) {
		return nullptr;
	}

	vkb::Instance vkb_instance = inst_ret.value();

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

	s_context->graphics_queue = Queue{
		.queue = vkb_device.get_queue(vkb::QueueType::graphics).value(),
		.queue_family =
				vkb_device.get_queue_index(vkb::QueueType::graphics).value(),
	};

	s_context->present_queue = Queue{
		.queue = vkb_device.get_queue(vkb::QueueType::present).value(),
		.queue_family =
				vkb_device.get_queue_index(vkb::QueueType::present).value(),
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

	return Context(s_context);
}

void VulkanRenderBackend::shutdown(Context device) {
	deletion_queue.flush();
	delete s_context;
}
