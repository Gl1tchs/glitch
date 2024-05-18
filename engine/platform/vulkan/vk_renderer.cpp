#include "platform/vulkan/vk_renderer.h"

#include "core/application.h"
#include "core/window.h"
#include "platform/vulkan/vk_commands.h"
#include "platform/vulkan/vk_init.h"
#include "platform/vulkan/vk_pipeline.h"

#include <VkBootstrap.h>
#include <vulkan/vulkan_core.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

// temp
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
// end temp

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
			{ { 0.5f, 0.5f, 0.0f }, 1.0f, { 0.0f, 0.0f, 0.0f }, 1.0f,
					{ 0.9f, 0.6f, 0.34f, 1.0f } },
			{ { 0.5f, -0.5f, 0.0f }, 1.0f, { 0.0f, 0.0f, 0.0f }, 0.0f,
					{ 0.2f, 0.4f, 0.8f, 1.0f } },
			{ { -0.5f, -0.5f, 0.0f }, 0.0f, { 0.0f, 0.0f, 0.0f }, 0.0f,
					{ 0.4f, 0.8f, 0.6f, 1.0f } },
			{ { -0.5f, 0.5f, 0.0f }, 0.0f, { 0.0f, 0.0f, 0.0f }, 1.0f,
					{ 0.8f, 0.2f, 0.4f, 1.0f } },
		};

		const std::vector<uint32_t> indices = { 0, 1, 2, 2, 3, 0 };

		uint32_t vertex_buffer_size = vertices.size() * sizeof(Vertex);
		uint32_t index_buffer_size = indices.size() * sizeof(uint32_t);

		vertex_buffer =
				VulkanBuffer::create(context.allocator, vertex_buffer_size,
						VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
								VK_BUFFER_USAGE_TRANSFER_DST_BIT |
								VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
						VMA_MEMORY_USAGE_GPU_ONLY);

		// get the address
		VkBufferDeviceAddressInfo address_info = {
			.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
			.buffer = vertex_buffer.buffer,
		};
		vertex_buffer_address =
				vkGetBufferDeviceAddress(context.device, &address_info);

		index_buffer =
				VulkanBuffer::create(context.allocator, index_buffer_size,
						VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
								VK_BUFFER_USAGE_TRANSFER_DST_BIT,
						VMA_MEMORY_USAGE_GPU_ONLY);

		deletion_queue.push_function([this]() {
			VulkanBuffer::destroy(context.allocator, vertex_buffer);
			VulkanBuffer::destroy(context.allocator, index_buffer);
		});

		VulkanBuffer staging = VulkanBuffer::create(context.allocator,
				vertex_buffer_size + index_buffer_size,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

		void* data = staging.allocation->GetMappedData();

		// copy vertex data
		memcpy(data, vertices.data(), vertex_buffer_size);
		// copy index data
		memcpy((char*)data + vertex_buffer_size, indices.data(),
				index_buffer_size);

		immediate_submit([&](VulkanCommandBuffer& cmd) {
			VkBufferCopy vertex_copy = {
				.srcOffset = 0,
				.dstOffset = 0,
				.size = vertex_buffer_size,
			};
			cmd.copy_buffer(staging, vertex_buffer, 1, &vertex_copy);

			VkBufferCopy index_copy = {
				.srcOffset = vertex_buffer_size,
				.dstOffset = 0,
				.size = index_buffer_size,
			};
			cmd.copy_buffer(staging, index_buffer, 1, &index_copy);
		});

		VulkanBuffer::destroy(context.allocator, staging);

		std::vector<VulkanDescriptorAllocator::PoolSizeRatio> pool_sizes = {
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 },
		};

		descriptor_allocator.init(context.device, 10, pool_sizes);

		DescriptorLayoutBuilder layout_builder;
		layout_builder.add_binding(
				0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

		descriptor_layout = layout_builder.build(
				context.device, VK_SHADER_STAGE_FRAGMENT_BIT);

		descriptor_set = descriptor_allocator.allocate(
				context.device, descriptor_layout);

		VkPushConstantRange push_constant = {
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
			.offset = 0,
			.size = sizeof(PushConstants),
		};

		VulkanPipelineLayoutCreateInfo layout_info = {
			.push_constant_count = 1,
			.push_constants = &push_constant,
			.descriptor_set_count = 1,
			.descriptor_sets = &descriptor_layout,
		};
		layout = VulkanPipelineLayout::create(context.device, &layout_info);

		VkShaderModule frag;
		bool shader_success =
				vk_load_shader_module(context.device, "lit.frag.spv", &frag);
		GL_ASSERT(shader_success);

		VkShaderModule vert;
		shader_success =
				vk_load_shader_module(context.device, "lit.vert.spv", &vert);
		GL_ASSERT(shader_success);

		VulkanPipelineCreateInfo pipeline_info;
		pipeline_info.set_shaders(vert, frag);
		pipeline_info.set_input_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
		pipeline_info.set_polygon_mode(VK_POLYGON_MODE_FILL);
		pipeline_info.set_cull_mode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
		pipeline_info.set_multisampling_none();
		// pipeline_info.disable_blending();
		pipeline_info.enable_blending(VulkanBlendingMode::ADDITIVE);
		pipeline_info.enable_depthtest(true, VK_COMPARE_OP_GREATER_OR_EQUAL);
		pipeline_info.set_color_attachment_format(draw_image.image_format);
		pipeline_info.set_depth_format(depth_image.image_format);

		pipeline =
				VulkanPipeline::create(context.device, &pipeline_info, &layout);

		vkDestroyShaderModule(context.device, frag, nullptr);
		vkDestroyShaderModule(context.device, vert, nullptr);

		deletion_queue.push_function([this]() {
			VulkanPipelineLayout::destroy(context.device, layout);
			VulkanPipeline::destroy(context.device, pipeline);
		});

		VkSamplerCreateInfo sampl = {
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.magFilter = VK_FILTER_LINEAR,
			.minFilter = VK_FILTER_LINEAR,
		};

		vkCreateSampler(context.device, &sampl, nullptr, &sampler_linear);

		VkExtent3D texture_extent = {
			.width = 0,
			.height = 0,
			.depth = 1,
		};

		int channel_count;
		uint8_t* image_data = stbi_load("assets/t_texture.png",
				(int*)&texture_extent.width, (int*)&texture_extent.height,
				&channel_count, STBI_rgb_alpha);

		texture_image = VulkanImage::create(context, image_data, texture_extent,
				VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);

		DescriptorWriter writer;
		writer.clear();
		writer.write_image(0, texture_image.image_view, sampler_linear,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		writer.update_set(context.device, descriptor_set);

		stbi_image_free(image_data);

		deletion_queue.push_function([this]() {
			descriptor_allocator.destroy_pools(context.device);

			VulkanImage::destroy(context, texture_image);
			vkDestroySampler(context.device, sampler_linear, nullptr);
		});
	}
	// end temp
}

VulkanRenderer::~VulkanRenderer() {
	vkDeviceWaitIdle(context.device);

	deletion_queue.flush();
}

VulkanRenderer* VulkanRenderer::get_instance() { return s_instance; }

void VulkanRenderer::draw() {
	VK_CHECK(vkWaitForFences(context.device, 1,
			&get_current_frame().render_fence, true, UINT64_MAX));

	get_current_frame().deletion_queue.flush();
	// get_current_frame().frame_descriptors.clear_pools(context.device);

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
	VK_CHECK(vkResetFences(
			context.device, 1, &get_current_frame().render_fence));

	// naming it cmd for shorter writing
	VulkanCommandBuffer cmd = get_current_frame().main_command_buffer;

	// now that we are sure that the commands finished executing, we can safely
	// reset the command buffer to begin recording again.
	cmd.reset();

	draw_extent.width = std::min(swapchain->get_extent().width,
								draw_image.image_extent.width) *
			render_scale;
	draw_extent.height = std::min(swapchain->get_extent().height,
								 draw_image.image_extent.height) *
			render_scale;

	// begin the command buffer recording. We will use this command buffer
	// exactly once, so we want to let vulkan know that
	cmd.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	{
		// clear color
		cmd.transition_image(
				draw_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

		VkClearColorValue clear_color = { { 0.1f, 0.1f, 0.1f, 1.0f } };

		cmd.clear_color_image(
				draw_image.image, VK_IMAGE_LAYOUT_GENERAL, &clear_color);

		// draw geometry
		cmd.transition_image(draw_image, VK_IMAGE_LAYOUT_GENERAL,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		cmd.transition_image(depth_image, VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

		_geometry_pass(cmd);

		// transition the draw image and the swapchain image into their correct
		// transfer layouts
		cmd.transition_image(draw_image,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		cmd.transition_image(swapchain->get_image(swapchain_image_index),
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		// execute a copy from the draw image into the swapchain
		cmd.copy_image_to_image(draw_image.image,
				swapchain->get_image(swapchain_image_index), draw_extent,
				swapchain->get_extent());

		// set swapchain image layout to present so we can show it on the
		// screen
		cmd.transition_image(swapchain->get_image(swapchain_image_index),
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	}
	cmd.end();

	_present_image(cmd, swapchain_image_index);

	frame_number++;
}

void VulkanRenderer::immediate_submit(
		std::function<void(VulkanCommandBuffer& cmd)>&& function) {
	VK_CHECK(vkResetFences(context.device, 1, &imm_fence));

	imm_command_buffer.reset();

	VkCommandBufferBeginInfo cmd_begin_info = vkinit::command_buffer_begin_info(
			VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	imm_command_buffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	{
		// run the command
		function(imm_command_buffer);
	}
	imm_command_buffer.end();

	// submit command buffer to the queue and execute it.
	// imm_fence will now block until the graphic commands finish execution
	imm_command_buffer.submit(context.graphics_queue, imm_fence);

	// wait till the operation finishes
	VK_CHECK(vkWaitForFences(context.device, 1, &imm_fence, true, UINT64_MAX));
}

void VulkanRenderer::_geometry_pass(VulkanCommandBuffer& cmd) {
	// begin a render pass  connected to our draw image
	VkRenderingAttachmentInfo color_attachment = vkinit::attachment_info(
			draw_image.image_view, nullptr, VK_IMAGE_LAYOUT_GENERAL);
	VkRenderingAttachmentInfo depth_attachment = vkinit::depth_attachment_info(
			depth_image.image_view, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

	cmd.begin_rendering(draw_extent, &color_attachment, &depth_attachment);
	{
		cmd.bind_pipeline(pipeline);

		// set dynamic viewport and scissor
		cmd.set_viewport(
				{ (float)draw_extent.width, (float)draw_extent.height });
		cmd.set_scissor(draw_extent);

		cmd.bind_descriptor_sets(layout, 0, 1, &descriptor_set);

		// draw geometry
		PushConstants push_constants = {
			.vertex_buffer = vertex_buffer_address,
		};
		cmd.push_constants(layout, VK_SHADER_STAGE_VERTEX_BIT, 0,
				sizeof(PushConstants), &push_constants);

		cmd.bind_index_buffer(index_buffer, 0, VK_INDEX_TYPE_UINT32);
		cmd.draw_indexed(6);
	}
	cmd.end_rendering();
}

void VulkanRenderer::_present_image(
		VulkanCommandBuffer& cmd, uint32_t swapchain_image_index) {
	VkSemaphoreSubmitInfo wait_info = vkinit::semaphore_submit_info(
			VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
			get_current_frame().swapchain_semaphore);
	VkSemaphoreSubmitInfo signal_info =
			vkinit::semaphore_submit_info(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
					get_current_frame().render_semaphore);

	// submit command buffer to the queue and execute it.
	// render_fence will now block until the graphic commands finish
	// execution
	cmd.submit(context.graphics_queue, get_current_frame().render_fence,
			&wait_info, &signal_info);

	VkPresentInfoKHR present_info = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.pNext = nullptr,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &get_current_frame().render_semaphore,
		.swapchainCount = 1,
		.pSwapchains = swapchain->get_swapchain(),
		.pImageIndices = &swapchain_image_index,
	};

	if (VkResult res = vkQueuePresentKHR(context.graphics_queue, &present_info);
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

	context.instance = vkb_instance.instance;
	context.debug_messenger = vkb_instance.debug_messenger;

	glfwCreateWindowSurface(context.instance,
			(GLFWwindow*)window->get_native_window(), nullptr,
			&context.surface);

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
					.set_surface(context.surface)
					.select()
					.value();

	// create the final vulkan context.device
	vkb::DeviceBuilder device_builder{ physical_device };
	vkb::Device vkb_device = device_builder.build().value();

	context.chosen_gpu = vkb_device.physical_device;
	context.device = vkb_device.device;

	context.graphics_queue =
			vkb_device.get_queue(vkb::QueueType::graphics).value();
	context.graphics_queue_family =
			vkb_device.get_queue_index(vkb::QueueType::graphics).value();

	deletion_queue.push_function([this]() {
		vkDestroySurfaceKHR(context.instance, context.surface, nullptr);
		vkDestroyDevice(context.device, nullptr);

		vkb::destroy_debug_utils_messenger(
				context.instance, context.debug_messenger, nullptr);
		vkDestroyInstance(context.instance, nullptr);
	});

	VmaAllocatorCreateInfo allocator_info = {
		.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
		.physicalDevice = context.chosen_gpu,
		.device = context.device,
		.instance = context.instance,
	};
	vmaCreateAllocator(&allocator_info, &context.allocator);

	deletion_queue.push_function(
			[this]() { vmaDestroyAllocator(context.allocator); });

	// get information about the context.device
	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(context.chosen_gpu, &properties);

	GL_LOG_INFO("Vulkan Initialized:");
	GL_LOG_INFO("Device: {}", properties.deviceName);
	GL_LOG_INFO("API: {}.{}.{}", VK_VERSION_MAJOR(properties.apiVersion),
			VK_VERSION_MINOR(properties.apiVersion),
			VK_VERSION_PATCH(properties.apiVersion));
}

void VulkanRenderer::_init_swapchain() {
	const auto window_size = window->get_size();
	swapchain = create_ref<VulkanSwapchain>(
			context.device, context.chosen_gpu, context.surface, window_size);

	deletion_queue.push_function([this]() { swapchain.reset(); });

	// prepare drawing images
	VkExtent3D draw_image_extent = {
		(uint32_t)window_size.x,
		(uint32_t)window_size.y,
		1,
	};

	draw_image = VulkanImage::create(context, draw_image_extent,
			VK_FORMAT_R16G16B16A16_SFLOAT,
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
					VK_IMAGE_USAGE_STORAGE_BIT |
					VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

	depth_image = VulkanImage::create(context, draw_image_extent,
			VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

	deletion_queue.push_function([this]() {
		VulkanImage::destroy(context, draw_image);
		VulkanImage::destroy(context, depth_image);
	});
}

void VulkanRenderer::_init_commands() {
	VkCommandPoolCreateInfo command_pool_info =
			vkinit::command_pool_create_info(context.graphics_queue_family,
					VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

	// create frame pools and buffers
	for (int i = 0; i < FRAME_OVERLAP; i++) {
		VulkanFrameData& frame = frames[i];

		frame.command_pool =
				VulkanCommandPool::create(context.device, &command_pool_info);
		// command pool cleanup
		deletion_queue.push_function([this, &frame]() {
			VulkanCommandPool::destroy(context.device, frame.command_pool);
		});

		frame.main_command_buffer =
				frame.command_pool.allocate_buffer(context.device);
	}

	// create immediate commands
	imm_command_pool =
			VulkanCommandPool::create(context.device, &command_pool_info);

	// allocate the command buffer for immediate submits
	imm_command_buffer = imm_command_pool.allocate_buffer(context.device);

	deletion_queue.push_function([this]() {
		VulkanCommandPool::destroy(context.device, imm_command_pool);
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
		// This is very important, as it allows us to wait on a freshly
		// created fence without causing errors. If we did not have that
		// bit, when we call into WaitFences the first frame, before the gpu
		// is doing work, the thread will be blocked.
		VK_CHECK(vkCreateFence(
				context.device, &fence_info, nullptr, &frames[i].render_fence));

		VK_CHECK(vkCreateSemaphore(context.device, &semaphore_info, nullptr,
				&frames[i].swapchain_semaphore));
		VK_CHECK(vkCreateSemaphore(context.device, &semaphore_info, nullptr,
				&frames[i].render_semaphore));

		// sync object cleanup
		deletion_queue.push_function([this, i]() {
			vkDestroyFence(context.device, frames[i].render_fence, nullptr);
			vkDestroySemaphore(
					context.device, frames[i].render_semaphore, nullptr);
			vkDestroySemaphore(
					context.device, frames[i].swapchain_semaphore, nullptr);
		});
	}

	// immediate sync structures
	VK_CHECK(vkCreateFence(context.device, &fence_info, nullptr, &imm_fence));
	deletion_queue.push_function(
			[this]() { vkDestroyFence(context.device, imm_fence, nullptr); });
}
