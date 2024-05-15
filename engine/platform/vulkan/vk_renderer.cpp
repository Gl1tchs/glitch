#include "platform/vulkan/vk_renderer.h"

#include "core/application.h"
#include "core/window.h"
#include "platform/vulkan/vk_init.h"

#include <VkBootstrap.h>
#include <vulkan/vulkan_core.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

VulkanRenderer* VulkanRenderer::s_instance = nullptr;

VulkanRenderer::VulkanRenderer(Ref<Window> window) : window(window) {
	GL_ASSERT(s_instance == nullptr);
	s_instance = this;

	_init_vulkan();
	_init_swapchain();
	_init_commands();
	_init_sync_structures();
}

VulkanRenderer::~VulkanRenderer() {
	vkDeviceWaitIdle(device);

	deletion_queue.flush();
}

VulkanRenderer* VulkanRenderer::get_instance() { return s_instance; }

void VulkanRenderer::draw() {
	VK_CHECK(vkWaitForFences(
			device, 1, &get_current_frame().render_fence, true, UINT64_MAX));

	get_current_frame().deletion_queue.flush();
	// get_current_frame().frame_descriptors.clear_pools(device);

	// request image from the swapchain
	uint32_t swapchain_image_index;

	if (VkResult res = swapchain->request_next_image(
				get_current_frame().swapchain_semaphore,
				&swapchain_image_index);
			res == VK_ERROR_OUT_OF_DATE_KHR) {
		// resize the swapchain on the next frame
		Application::get_instance()->enqueue_main_thread(
				[this]() { swapchain->resize(window->get_size()); });
		return;
	}

	frame_number++;
}

void VulkanRenderer::immediate_submit(
		std::function<void(VkCommandBuffer cmd)>&& function) {
	VK_CHECK(vkResetFences(device, 1, &imm_fence));
	VK_CHECK(vkResetCommandBuffer(imm_command_buffer, 0));

	VkCommandBuffer cmd = imm_command_buffer;

	VkCommandBufferBeginInfo cmd_begin_info = vkinit::command_buffer_begin_info(
			VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VK_CHECK(vkBeginCommandBuffer(imm_command_buffer, &cmd_begin_info));
	{
		// run the command
		function(cmd);
	}
	VK_CHECK(vkEndCommandBuffer(cmd));

	VkCommandBufferSubmitInfo cmd_info =
			vkinit::command_buffer_submit_info(cmd);
	VkSubmitInfo2 submit = vkinit::submit_info(&cmd_info, nullptr, nullptr);

	// submit command buffer to the queue and execute it.
	// imm_fence will now block until the graphic commands finish execution
	VK_CHECK(vkQueueSubmit2(graphics_queue, 1, &submit, imm_fence));

	// wait till the operation finishes
	VK_CHECK(vkWaitForFences(device, 1, &imm_fence, true, UINT64_MAX));
}

void VulkanRenderer::_init_vulkan() {
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
	vkb::PhysicalDevice physicaldevice =
			selector.set_minimum_version(1, 3)
					.set_required_features_13(features13)
					.set_required_features_12(features12)
					.set_surface(surface)
					.select()
					.value();

	// create the final vulkan device
	vkb::DeviceBuilder device_builder{ physicaldevice };
	vkb::Device vkbdevice = device_builder.build().value();

	chosen_gpu = physicaldevice.physical_device;
	device = vkbdevice.device;

	graphics_queue = vkbdevice.get_queue(vkb::QueueType::graphics).value();
	graphics_queue_family =
			vkbdevice.get_queue_index(vkb::QueueType::graphics).value();

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

void VulkanRenderer::_init_swapchain() {
	const auto window_size = window->get_size();
	swapchain = create_ref<VulkanSwapchain>(
			device, chosen_gpu, surface, window_size);

	deletion_queue.push_function([this]() { swapchain.reset(); });

	// prepare drawing images
	VkExtent3D draw_image_extent = {
		(uint32_t)window_size.x,
		(uint32_t)window_size.y,
		1,
	};

	draw_image = VulkanImage::create(device, allocator, draw_image_extent,
			VK_FORMAT_R16G16B16A16_SFLOAT,
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
					VK_IMAGE_USAGE_STORAGE_BIT |
					VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

	depth_image = VulkanImage::create(device, allocator, draw_image_extent,
			VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

	deletion_queue.push_function([this]() {
		VulkanImage::destroy(device, allocator, draw_image);
		VulkanImage::destroy(device, allocator, depth_image);
	});
}

void VulkanRenderer::_init_commands() {
	VkCommandPoolCreateInfo command_pool_info =
			vkinit::command_pool_create_info(graphics_queue_family,
					VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

	// create frame pools and buffers
	for (int i = 0; i < FRAME_OVERLAP; i++) {
		VK_CHECK(vkCreateCommandPool(
				device, &command_pool_info, nullptr, &frames[i].command_pool));
		// command pool cleanup
		deletion_queue.push_function([this, i]() {
			vkDestroyCommandPool(device, frames[i].command_pool, nullptr);
		});

		VkCommandBufferAllocateInfo cmd_alloc_info =
				vkinit::command_buffer_allocate_info(frames[i].command_pool, 1);

		VK_CHECK(vkAllocateCommandBuffers(
				device, &cmd_alloc_info, &frames[i].main_command_buffer));
	}

	// create immediate commands

	VK_CHECK(vkCreateCommandPool(
			device, &command_pool_info, nullptr, &imm_command_pool));

	// allocate the command buffer for immediate submits
	VkCommandBufferAllocateInfo cmd_alloc_info =
			vkinit::command_buffer_allocate_info(imm_command_pool, 1);

	VK_CHECK(vkAllocateCommandBuffers(
			device, &cmd_alloc_info, &imm_command_buffer));

	deletion_queue.push_function([this]() {
		vkDestroyCommandPool(device, imm_command_pool, nullptr);
	});
}

void VulkanRenderer::_init_sync_structures() {
	//create synchronization structures
	//one fence to control when the gpu has finished rendering the frame,
	//and 2 semaphores to synchronize rendering with swapchain
	VkFenceCreateInfo fence_info =
			vkinit::fence_create_info(VK_FENCE_CREATE_SIGNALED_BIT);
	VkSemaphoreCreateInfo semaphore_info = vkinit::semaphore_create_info();

	for (int i = 0; i < FRAME_OVERLAP; i++) {
		// On the fence, we are using the flag VK_FENCE_CREATE_SIGNALED_BIT.
		// This is very important, as it allows us to wait on a freshly created
		// fence without causing errors. If we did not have that bit, when we
		// call into WaitFences the first frame, before the gpu is doing work,
		// the thread will be blocked.
		VK_CHECK(vkCreateFence(
				device, &fence_info, nullptr, &frames[i].render_fence));

		VK_CHECK(vkCreateSemaphore(device, &semaphore_info, nullptr,
				&frames[i].swapchain_semaphore));
		VK_CHECK(vkCreateSemaphore(
				device, &semaphore_info, nullptr, &frames[i].render_semaphore));

		// sync object cleanup
		deletion_queue.push_function([this, i]() {
			vkDestroyFence(device, frames[i].render_fence, nullptr);
			vkDestroySemaphore(device, frames[i].render_semaphore, nullptr);
			vkDestroySemaphore(device, frames[i].swapchain_semaphore, nullptr);
		});
	}

	// immediate sync structures
	VK_CHECK(vkCreateFence(device, &fence_info, nullptr, &imm_fence));
	deletion_queue.push_function(
			[this]() { vkDestroyFence(device, imm_fence, nullptr); });
}
