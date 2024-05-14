#include "renderer/vulkan/vk_context.h"

#include "VkBootstrap.h"
#include "core/window.h"

#include <vulkan/vulkan_core.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

void VulkanContext::init(Ref<Window> window) {
	vkb::InstanceBuilder builder;

	auto inst_ret =
			builder.set_app_name("glitch")
					// TODO do not activate debug messenger on DIST build
					.use_default_debug_messenger()
					.require_api_version(1, 3, 0)
					.build();

	vkb::Instance vkb_instance = inst_ret.value();

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

	vkb::PhysicalDeviceSelector selector{ vkb_instance };
	vkb::PhysicalDevice physical_device =
			selector.set_minimum_version(1, 3)
					.set_required_features_13(features13)
					.set_required_features_12(features12)
					.set_surface(surface)
					.select()
					.value();

	// create the final vulkan device
	vkb::DeviceBuilder device_builder{ physical_device };
	vkb::Device vkb_device = device_builder.build().value();

	chosen_gpu = physical_device.physical_device;
	device = vkb_device.device;

	graphics_queue = vkb_device.get_queue(vkb::QueueType::graphics).value();
	graphics_queue_family =
			vkb_device.get_queue_index(vkb::QueueType::graphics).value();

	deletion_queue.push_function([this]() {
		vkDestroySurfaceKHR(instance, surface, nullptr);
		vkDestroyDevice(device, nullptr);

		vkb::destroy_debug_utils_messenger(instance, debug_messenger, nullptr);
		vkDestroyInstance(instance, nullptr);
	});

	VmaAllocatorCreateInfo allocator_info = {
		.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
		.physicalDevice = chosen_gpu,
		.device = device,
		.instance = instance,
	};
	vmaCreateAllocator(&allocator_info, &allocator);

	deletion_queue.push_function([this]() { vmaDestroyAllocator(allocator); });

    // get information about the device
	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(chosen_gpu, &properties);

	GL_LOG_INFO("Vulkan Initialized:");
	GL_LOG_INFO("Device: {}", properties.deviceName);
	GL_LOG_INFO("API: {}.{}.{}", VK_VERSION_MAJOR(properties.apiVersion),
			VK_VERSION_MINOR(properties.apiVersion),
			VK_VERSION_PATCH(properties.apiVersion));
}

void VulkanContext::destroy() {
	//make sure the gpu has stopped doing its things
	vkDeviceWaitIdle(device);

	deletion_queue.flush();
}
