#include "platform/vulkan/vk_renderer.h"

#include "core/application.h"
#include "core/window.h"
#include "platform/vulkan/vk_init.h"
#include "platform/vulkan/vk_pipeline.h"

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

	// temp
	{
		const std::vector<Vertex> vertices = {
			{ { 0.5f, 0.5f, 0.0f }, 0.0f, { 0.0f, 0.0f, 0.0f }, 0.0f,
					{ 0.9f, 0.6f, 0.34f, 1.0f } },
			{ { 0.5f, -0.5f, 0.0f }, 0.0f, { 0.0f, 0.0f, 0.0f }, 1.0f,
					{ 0.2f, 0.4f, 0.8f, 1.0f } },
			{ { -0.5f, -0.5f, 0.0f }, 1.0f, { 0.0f, 0.0f, 0.0f }, 1.0f,
					{ 0.4f, 0.8f, 0.6f, 1.0f } },
			{ { -0.5f, 0.5f, 0.0f }, 1.0f, { 0.0f, 0.0f, 0.0f }, 0.0f,
					{ 0.8f, 0.2f, 0.4f, 1.0f } },
		};

		const std::vector<uint32_t> indices = { 0, 1, 2, 2, 3, 0 };

		uint32_t vertex_buffer_size = vertices.size() * sizeof(Vertex);
		uint32_t index_buffer_size = indices.size() * sizeof(uint32_t);

		vertex_buffer = VulkanBuffer::create(allocator, vertex_buffer_size,
				VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
						VK_BUFFER_USAGE_TRANSFER_DST_BIT |
						VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
				VMA_MEMORY_USAGE_GPU_ONLY);

		// get the address
		VkBufferDeviceAddressInfo address_info = {
			.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
			.buffer = vertex_buffer.buffer,
		};
		vertex_buffer_address = vkGetBufferDeviceAddress(device, &address_info);

		index_buffer = VulkanBuffer::create(allocator, index_buffer_size,
				VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
						VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				VMA_MEMORY_USAGE_GPU_ONLY);

		deletion_queue.push_function([this]() {
			VulkanBuffer::destroy(allocator, vertex_buffer);
			VulkanBuffer::destroy(allocator, index_buffer);
		});

		VulkanBuffer staging = VulkanBuffer::create(allocator,
				vertex_buffer_size + index_buffer_size,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

		void* data = staging.allocation->GetMappedData();

		// copy vertex data
		memcpy(data, vertices.data(), vertex_buffer_size);
		// copy index data
		memcpy((char*)data + vertex_buffer_size, indices.data(),
				index_buffer_size);

		immediate_submit([&](VkCommandBuffer cmd) {
			VkBufferCopy vertex_copy = {
				.srcOffset = 0,
				.dstOffset = 0,
				.size = vertex_buffer_size,
			};

			vkCmdCopyBuffer(
					cmd, staging.buffer, vertex_buffer.buffer, 1, &vertex_copy);

			VkBufferCopy index_copy = {
				.srcOffset = vertex_buffer_size,
				.dstOffset = 0,
				.size = index_buffer_size,
			};

			vkCmdCopyBuffer(
					cmd, staging.buffer, index_buffer.buffer, 1, &index_copy);
		});

		VulkanBuffer::destroy(allocator, staging);

		VkPushConstantRange push_constant = {
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
			.offset = 0,
			.size = sizeof(PushConstants),
		};

		std::vector<VkPushConstantRange> push_constants = { push_constant };

		VulkanPipelineLayoutCreateInfo layout_info = {
			.push_constants = push_constants,
		};
		layout = new VulkanPipelineLayout(device, &layout_info);

		VkShaderModule frag;
		bool shader_success =
				vk_load_shader_module(device, "lit.frag.spv", &frag);
		GL_ASSERT(shader_success);

		VkShaderModule vert;
		shader_success = vk_load_shader_module(device, "lit.vert.spv", &vert);
		GL_ASSERT(shader_success);

		VulkanPipelineCreateInfo pipeline_info;
		pipeline_info.set_shaders(vert, frag);
		pipeline_info.set_input_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
		pipeline_info.set_polygon_mode(VK_POLYGON_MODE_FILL);
		pipeline_info.set_cull_mode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
		pipeline_info.set_multisampling_none();
		pipeline_info.disable_blending();
		pipeline_info.enable_depthtest(true, VK_COMPARE_OP_GREATER_OR_EQUAL);
		pipeline_info.set_color_attachment_format(draw_image.image_format);
		pipeline_info.set_depth_format(depth_image.image_format);

		pipeline = new VulkanPipeline(device, &pipeline_info, layout);

		vkDestroyShaderModule(device, frag, nullptr);
		vkDestroyShaderModule(device, vert, nullptr);

		deletion_queue.push_function([this]() {
			delete layout;
			delete pipeline;
		});
	}
	// end temp
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

	// wait till we ensure that swapchain is not out of date
	VK_CHECK(vkResetFences(device, 1, &get_current_frame().render_fence));

	// naming it cmd for shorter writing
	VkCommandBuffer cmd = get_current_frame().main_command_buffer;

	// now that we are sure that the commands finished executing, we can safely
	// reset the command buffer to begin recording again.
	VK_CHECK(vkResetCommandBuffer(cmd, 0));

	// begin the command buffer recording. We will use this command buffer
	// exactly once, so we want to let vulkan know that
	VkCommandBufferBeginInfo cmd_begin_info = vkinit::command_buffer_begin_info(
			VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	draw_extent.width = std::min(swapchain->get_extent().width,
								draw_image.image_extent.width) *
			render_scale;
	draw_extent.height = std::min(swapchain->get_extent().height,
								 draw_image.image_extent.height) *
			render_scale;

	// start the command buffer recording
	VK_CHECK(vkBeginCommandBuffer(cmd, &cmd_begin_info));
	{
		// clear color
		VulkanImage::transition_image(cmd, draw_image.image,
				VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

		VkClearColorValue clear_color = { { 0.1f, 0.1f, 0.1f, 1.0f } };

		_clear_render_image(cmd, clear_color);

		// draw geometry
		VulkanImage::transition_image(cmd, draw_image.image,
				VK_IMAGE_LAYOUT_GENERAL,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		VulkanImage::transition_image(cmd, depth_image.image,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

		_geometry_pass(cmd);

		// transition the draw image and the swapchain image into their correct
		// transfer layouts
		VulkanImage::transition_image(cmd, draw_image.image,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		VulkanImage::transition_image(cmd,
				swapchain->get_image(swapchain_image_index),
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		// execute a copy from the draw image into the swapchain
		VulkanImage::copy_image_to_image(cmd, draw_image.image,
				swapchain->get_image(swapchain_image_index), draw_extent,
				swapchain->get_extent());

		// set swapchain image layout to present so we can show it on the screen
		VulkanImage::transition_image(cmd,
				swapchain->get_image(swapchain_image_index),
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	}
	VK_CHECK(vkEndCommandBuffer(cmd));

	_present_image(cmd, swapchain_image_index);

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

void VulkanRenderer::_clear_render_image(
		VkCommandBuffer cmd, VkClearColorValue clear_color) {
	VkImageSubresourceRange image_range = {
		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.levelCount = 1,
		.layerCount = 1,
	};
	vkCmdClearColorImage(cmd, draw_image.image, VK_IMAGE_LAYOUT_GENERAL,
			&clear_color, 1, &image_range);
}

void VulkanRenderer::_geometry_pass(VkCommandBuffer cmd) {
	// begin a render pass  connected to our draw image
	VkRenderingAttachmentInfo color_attachment = vkinit::attachment_info(
			draw_image.image_view, nullptr, VK_IMAGE_LAYOUT_GENERAL);
	VkRenderingAttachmentInfo depth_attachment = vkinit::depth_attachment_info(
			depth_image.image_view, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

	VkRenderingInfo render_info = vkinit::rendering_info(
			draw_extent, &color_attachment, &depth_attachment);
	vkCmdBeginRendering(cmd, &render_info);

	pipeline->bind(cmd);

	// set dynamic viewport and scissor
	VkViewport viewport = {};
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = draw_extent.width;
	viewport.height = draw_extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	vkCmdSetViewport(cmd, 0, 1, &viewport);

	VkRect2D scissor = {};
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent.width = draw_extent.width;
	scissor.extent.height = draw_extent.height;

	vkCmdSetScissor(cmd, 0, 1, &scissor);

	// draw geometry
	PushConstants push_constants = {
		.vertex_buffer = vertex_buffer_address,
	};
	vkCmdPushConstants(cmd, layout->layout, VK_SHADER_STAGE_VERTEX_BIT, 0,
			sizeof(PushConstants), &push_constants);

	vkCmdBindIndexBuffer(cmd, index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdDrawIndexed(cmd, 6, 1, 0, 0, 0);

	vkCmdEndRendering(cmd);
}

void VulkanRenderer::_present_image(
		VkCommandBuffer cmd, uint32_t swapchain_image_index) {
	VkCommandBufferSubmitInfo cmd_info =
			vkinit::command_buffer_submit_info(cmd);

	VkSemaphoreSubmitInfo wait_info = vkinit::semaphore_submit_info(
			VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
			get_current_frame().swapchain_semaphore);
	VkSemaphoreSubmitInfo signal_info =
			vkinit::semaphore_submit_info(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
					get_current_frame().render_semaphore);

	VkSubmitInfo2 submit =
			vkinit::submit_info(&cmd_info, &signal_info, &wait_info);

	// submit command buffer to the queue and execute it.
	// _renderFence will now block until the graphic commands finish execution
	VK_CHECK(vkQueueSubmit2(
			graphics_queue, 1, &submit, get_current_frame().render_fence));

	VkPresentInfoKHR present_info = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.pNext = nullptr,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &get_current_frame().render_semaphore,
		.swapchainCount = 1,
		.pSwapchains = swapchain->get_swapchain(),
		.pImageIndices = &swapchain_image_index,
	};

	if (VkResult res = vkQueuePresentKHR(graphics_queue, &present_info);
			res == VK_ERROR_OUT_OF_DATE_KHR) {
		// resize the swapchain on the next frame
		Application::get_instance()->enqueue_main_thread(
				[this]() { swapchain->resize(window->get_size()); });
	}
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
