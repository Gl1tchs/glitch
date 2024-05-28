#include "platform/vulkan/vk_renderer.h"

#include "gl/core/application.h"
#include "gl/core/window.h"
#include "gl/renderer/renderer.h"

#include "platform/vulkan/vk_commands.h"
#include "platform/vulkan/vk_compute.h"
#include "platform/vulkan/vk_context.h"
#include "platform/vulkan/vk_descriptors.h"
#include "platform/vulkan/vk_init.h"
#include "platform/vulkan/vk_material.h"
#include "platform/vulkan/vk_mesh.h"

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
	_init_descriptors();
	_init_samplers();
}

VulkanRenderer::~VulkanRenderer() {
	wait_for_device();

	for (auto& frame : frames) {
		frame.deletion_queue.flush();
	}

	deletion_queue.flush();
}

void VulkanRenderer::attach_camera(Camera* camera) { this->camera = camera; }

void VulkanRenderer::submit_mesh(Ref<Mesh> mesh, Ref<MaterialInstance> material,
		const InstanceSubmitData& data) {
	Ref<VulkanMesh> vk_mesh = std::dynamic_pointer_cast<VulkanMesh>(mesh);
	meshes_to_draw[material].push_back({ vk_mesh, data });
}

void VulkanRenderer::submit_compute_effect(Ref<ComputeEffect> effect) {
	Ref<VulkanComputeEffect> vk_effect =
			std::dynamic_pointer_cast<VulkanComputeEffect>(effect);
	compute_effects.push_back(vk_effect);
}

void VulkanRenderer::draw() {
	VK_CHECK(vkWaitForFences(context.device, 1,
			&get_current_frame().render_fence, true, UINT64_MAX));

	get_current_frame().deletion_queue.flush();
	get_current_frame().frame_descriptors.clear_pools(context.device);

	// request image from the swapchain
	uint32_t swapchain_image_index;
	if (VkResult res = swapchain->request_next_image(context,
				get_current_frame().swapchain_semaphore,
				&swapchain_image_index);
			res == VK_ERROR_OUT_OF_DATE_KHR) {
		// resize the swapchain on the next frame
		_request_resize();
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

	// set render scale
	const float render_scale = get_settings().render_scale;
	{
		draw_extent.width = std::min(swapchain->get_extent().width,
									draw_image->image_extent.width) *
				render_scale;
		draw_extent.height = std::min(swapchain->get_extent().height,
									 draw_image->image_extent.height) *
				render_scale;
	}

	// begin the command buffer recording. We will use
	// this command buffer exactly once, so we want to
	// let vulkan know that
	cmd.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	{
		// clear color
		cmd.transition_image(
				draw_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

		VkClearColorValue clear_color = { { 0.1f, 0.1f, 0.1f, 1.0f } };

		cmd.clear_color_image(
				draw_image->image, VK_IMAGE_LAYOUT_GENERAL, &clear_color);

		_compute_pass(cmd);

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
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		cmd.transition_image(swapchain->get_image(swapchain_image_index),
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		// execute a copy from the draw image into the swapchain
		cmd.copy_image_to_image(draw_image->image,
				swapchain->get_image(swapchain_image_index), draw_extent,
				swapchain->get_extent());

		// set swapchain image layout to present so we can show it on the
		// screen
		cmd.transition_image(swapchain->get_image(swapchain_image_index),
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	}
	cmd.end();

	// submit commands
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

	_present_image(cmd, swapchain_image_index);

	frame_number++;
}

void VulkanRenderer::wait_for_device() { vkDeviceWaitIdle(context.device); }

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

VulkanRenderer* VulkanRenderer::get_instance() { return s_instance; }

VulkanContext& VulkanRenderer::get_context() { return get_instance()->context; }

void VulkanRenderer::_geometry_pass(VulkanCommandBuffer& cmd) {
	// begin a render pass  connected to our draw image
	VkRenderingAttachmentInfo color_attachment = vkinit::attachment_info(
			draw_image->image_view, nullptr, VK_IMAGE_LAYOUT_GENERAL);

	VkRenderingAttachmentInfo depth_attachment = vkinit::depth_attachment_info(
			depth_image->image_view, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

	VulkanBuffer scene_data_buffer = VulkanBuffer::create(context.allocator,
			sizeof(VulkanSceneData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VMA_MEMORY_USAGE_CPU_TO_GPU);

	get_current_frame().deletion_queue.push_function([=, this]() {
		VulkanBuffer::destroy(context.allocator, scene_data_buffer);
	});

	VulkanSceneData* scene_uniform_data =
			(VulkanSceneData*)scene_data_buffer.allocation->GetMappedData();
	if (camera) {
		scene_uniform_data->view = camera->get_view_matrix();
		scene_uniform_data->proj = camera->get_projection_matrix();
		scene_uniform_data->view_proj =
				scene_uniform_data->proj * scene_uniform_data->view;
	}

	VkDescriptorSet global_descriptor =
			get_current_frame().frame_descriptors.allocate(
					context.device, context.scene_data_descriptor_layout);

	DescriptorWriter writer;
	writer.write_buffer(0, scene_data_buffer.buffer, sizeof(VulkanSceneData), 0,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	writer.update_set(context.device, global_descriptor);

	cmd.begin_rendering(draw_extent, &color_attachment, &depth_attachment);
	{
		// set dynamic viewport and scissor
		cmd.set_viewport(
				{ (float)draw_extent.width, (float)draw_extent.height });
		cmd.set_scissor(draw_extent);

		for (const auto& [material, meshes] : meshes_to_draw) {
			Ref<VulkanMaterialInstance> vk_material =
					std::dynamic_pointer_cast<VulkanMaterialInstance>(material);

			cmd.bind_pipeline(vk_material->pipeline->pipeline);

			cmd.bind_descriptor_sets(vk_material->pipeline->pipeline_layout, 0,
					1, &global_descriptor);
			cmd.bind_descriptor_sets(vk_material->pipeline->pipeline_layout, 1,
					1, &vk_material->descriptor_set);

			for (const auto& [mesh, data] : meshes) {
				// draw geometry
				DrawPushConstants push_constants = {
					.transform = data.transform,
					.vertex_buffer = mesh->vertex_buffer_address,
				};
				cmd.push_constants(vk_material->pipeline->pipeline_layout,
						VK_SHADER_STAGE_VERTEX_BIT, 0,
						sizeof(DrawPushConstants), &push_constants);

				cmd.bind_index_buffer(
						mesh->index_buffer, 0, VK_INDEX_TYPE_UINT32);
				cmd.draw_indexed(mesh->indices.size());
			}
		}
		cmd.end_rendering();
	}

	meshes_to_draw.clear();
}

void VulkanRenderer::_compute_pass(VulkanCommandBuffer& cmd) {
	VulkanBuffer compute_ub = VulkanBuffer::create(context.allocator,
			sizeof(ComputeUB), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VMA_MEMORY_USAGE_CPU_TO_GPU);

	get_current_frame().deletion_queue.push_function([=, this]() {
		VulkanBuffer::destroy(context.allocator, compute_ub);
	});

	ComputeUB* compute_ub_data =
			(ComputeUB*)compute_ub.allocation->GetMappedData();
	compute_ub_data->time = timer.get_elapsed_seconds();

	VkDescriptorSet compute_descriptor =
			get_current_frame().frame_descriptors.allocate(
					context.device, context.compute_data_descriptor_layout);

	DescriptorWriter writer;
	writer.write_buffer(0, compute_ub.buffer, sizeof(ComputeUB), 0,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	writer.update_set(context.device, compute_descriptor);

	for (const auto& compute_effect : compute_effects) {
		cmd.bind_pipeline(compute_effect->pipeline);

		cmd.bind_descriptor_sets(compute_effect->pipeline_layout, 0, 1,
				&context.compute_descriptor_set,
				VK_PIPELINE_BIND_POINT_COMPUTE);
		cmd.bind_descriptor_sets(compute_effect->pipeline_layout, 1, 1,
				&compute_descriptor, VK_PIPELINE_BIND_POINT_COMPUTE);

		cmd.dispatch(compute_effect->group_count.x,
				compute_effect->group_count.y, compute_effect->group_count.z);
	}

	compute_effects.clear();
}

void VulkanRenderer::_present_image(
		VulkanCommandBuffer& cmd, uint32_t swapchain_image_index) {
	VkPresentInfoKHR present_info = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.pNext = nullptr,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &get_current_frame().render_semaphore,
		.swapchainCount = 1,
		.pSwapchains = swapchain->get_swapchain(),
		.pImageIndices = &swapchain_image_index,
	};

	if (VkResult res = vkQueuePresentKHR(context.present_queue, &present_info);
			res == VK_ERROR_OUT_OF_DATE_KHR) {
		// resize the swapchain on the next frame
		_request_resize();
	}
}

inline VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
		VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
		VkDebugUtilsMessageTypeFlagsEXT message_type,
		const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
		void* user_data) {
	auto ms = vkb::to_string_message_severity(message_severity);
	auto mt = vkb::to_string_message_type(message_type);
	GL_LOG_TRACE("[{}: {}]\n{}", ms, mt, callback_data->pMessage);

	return VK_FALSE;
}

void VulkanRenderer::_init_vulkan() {
	vkb::InstanceBuilder builder;

	auto inst_ret =
			builder.set_app_name("glitch")
					// TODO do not activate debug messenger on DIST build
					.enable_validation_layers()
					.set_debug_callback(vk_debug_callback)
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

	context.present_queue =
			vkb_device.get_queue(vkb::QueueType::present).value();
	context.present_queue_family =
			vkb_device.get_queue_index(vkb::QueueType::present).value();

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
	swapchain = VulkanSwapchain::create(context, window_size);

	deletion_queue.push_function(
			[this]() { VulkanSwapchain::destroy(context, swapchain.get()); });

	// prepare drawing images
	VkExtent3D draw_image_extent = {
		(uint32_t)window_size.x,
		(uint32_t)window_size.y,
		1,
	};

	context.color_attachment_format = VK_FORMAT_R16G16B16A16_SFLOAT;

	VulkanImageCreateInfo draw_image_info = {
		.format = context.color_attachment_format,
		.size = draw_image_extent,
		.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
				VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT |
				VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
	};
	draw_image = VulkanImage::create(context, &draw_image_info);

	context.depth_attachment_format = VK_FORMAT_D32_SFLOAT;

	VulkanImageCreateInfo depth_image_info = {
		.format = context.depth_attachment_format,
		.size = draw_image_extent,
		.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
	};
	depth_image = VulkanImage::create(context, &depth_image_info);

	deletion_queue.push_function([this]() {
		VulkanImage::destroy(context, draw_image.get());
		VulkanImage::destroy(context, depth_image.get());
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

void VulkanRenderer::_init_descriptors() {
	// geometry descriptors
	{
		std::vector<VulkanDescriptorAllocator::PoolSizeRatio> pool_sizes = {
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2 },
		};

		context.descriptor_allocator.init(context.device, 10, pool_sizes);

		deletion_queue.push_function([this]() {
			context.descriptor_allocator.destroy_pools(context.device);
		});

		for (int i = 0; i < FRAME_OVERLAP; i++) {
			// create a descriptor pool
			std::vector<VulkanDescriptorAllocator::PoolSizeRatio>
					frame_sizes = {
						{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3 },
					};

			frames[i].frame_descriptors = VulkanDescriptorAllocator{};
			frames[i].frame_descriptors.init(context.device, 1000, frame_sizes);

			deletion_queue.push_function([this, i]() {
				frames[i].frame_descriptors.destroy_pools(context.device);
			});
		}

		{
			DescriptorLayoutBuilder builder;
			builder.add_binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
			context.scene_data_descriptor_layout = builder.build(context.device,
					VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

			deletion_queue.push_function([this]() {
				vkDestroyDescriptorSetLayout(context.device,
						context.scene_data_descriptor_layout, nullptr);
			});
		}
	}

	// compute descriptors
	{
		std::vector<VulkanDescriptorAllocator::PoolSizeRatio> sizes = {
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 },
		};

		context.compute_descriptor_allocator.init(context.device, 10, sizes);

		deletion_queue.push_function([this]() {
			context.compute_descriptor_allocator.destroy_pools(context.device);
		});

		{
			DescriptorLayoutBuilder builder;
			builder.add_binding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
			context.compute_descriptor_layout =
					builder.build(context.device, VK_SHADER_STAGE_COMPUTE_BIT);

			deletion_queue.push_function([this]() {
				vkDestroyDescriptorSetLayout(context.device,
						context.compute_descriptor_layout, nullptr);
			});
		}

		context.compute_descriptor_set =
				context.compute_descriptor_allocator.allocate(
						context.device, context.compute_descriptor_layout);

		{
			DescriptorWriter writer;
			writer.write_image(0, draw_image->image_view, VK_NULL_HANDLE,
					VK_IMAGE_LAYOUT_GENERAL, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
			writer.update_set(context.device, context.compute_descriptor_set);
		}

		{
			DescriptorLayoutBuilder builder;
			builder.add_binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
			context.compute_data_descriptor_layout =
					builder.build(context.device, VK_SHADER_STAGE_COMPUTE_BIT);

			deletion_queue.push_function([this]() {
				vkDestroyDescriptorSetLayout(context.device,
						context.compute_data_descriptor_layout, nullptr);
			});
		}
	}
}

void VulkanRenderer::_init_samplers() {
	VkSamplerCreateInfo sampler_info = {
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.magFilter = VK_FILTER_LINEAR,
		.minFilter = VK_FILTER_LINEAR,
	};

	vkCreateSampler(
			context.device, &sampler_info, nullptr, &context.linear_sampler);

	sampler_info.magFilter = VK_FILTER_NEAREST;
	sampler_info.minFilter = VK_FILTER_NEAREST;

	vkCreateSampler(
			context.device, &sampler_info, nullptr, &context.nearest_sampler);

	deletion_queue.push_function([this]() {
		vkDestroySampler(context.device, context.linear_sampler, nullptr);
		vkDestroySampler(context.device, context.nearest_sampler, nullptr);
	});
}

void VulkanRenderer::_request_resize() {
	Application::get_instance()->enqueue_main_thread(
			[this]() { swapchain->resize(context, window->get_size()); });
}
