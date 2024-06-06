#include "renderer/renderer.h"

#include "core/application.h"

#include "renderer/node.h"
#include "renderer/types.h"

#include "platform/vulkan/vk_backend.h"
#include "platform/vulkan/vk_commands.h"
#include "platform/vulkan/vk_image.h"
#include "platform/vulkan/vk_queue.h"
#include "platform/vulkan/vk_swapchain.h"
#include "platform/vulkan/vk_sync.h"

[[nodiscard]] GraphicsAPI find_proper_api() noexcept {
	return GRAPHICS_API_VULKAN;
}

static GraphicsAPI s_api;

Renderer::Renderer(Ref<Window> p_window) : window(p_window) {
	s_api = find_proper_api();

	backend = create_ref<VulkanRenderBackend>();
	context = backend->init(p_window);

	swapchain = vk::swapchain_create();
	vk::swapchain_resize(context,
			backend->get_command_queue(QUEUE_TYPE_PRESENT), swapchain,
			p_window->get_size());

	draw_image = vk::image_create(context, DATA_FORMAT_R16G16B16A16_SFLOAT,
			p_window->get_size(), nullptr,
			IMAGE_USAGE_TRANSFER_SRC_BIT | IMAGE_USAGE_TRANSFER_DST_BIT |
					IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

	for (uint8_t i = 0; i < SWAPCHAIN_BUFFER_SIZE; i++) {
		FrameData& frame_data = frames[i];

		frame_data.command_pool = vk::command_pool_create(
				context, backend->get_command_queue(QUEUE_TYPE_GRAPHICS));
		frame_data.command_buffer =
				vk::command_pool_allocate(context, frame_data.command_pool);

		frame_data.image_available_semaphore = vk::semaphore_create(context);
		frame_data.render_finished_semaphore = vk::semaphore_create(context);

		frame_data.render_fence = vk::fence_create(context);
	}
}

Renderer::~Renderer() {
	backend->wait_for_device();

	_destroy_scene_graph();

	for (uint8_t i = 0; i < SWAPCHAIN_BUFFER_SIZE; i++) {
		FrameData& frame_data = frames[i];

		vk::command_pool_free(context, frame_data.command_pool);

		vk::semaphore_free(context, frame_data.image_available_semaphore);
		vk::semaphore_free(context, frame_data.render_finished_semaphore);

		vk::fence_free(context, frame_data.render_fence);
	}

	vk::image_free(context, draw_image);
	vk::swapchain_free(context, swapchain);

	backend->shutdown(context);
}

void Renderer::wait_and_render() {
	vk::fence_wait(context, _get_current_frame().render_fence);

	Optional<Image> swapchain_image = vk::swapchain_acquire_image(
			context, swapchain, _get_current_frame().image_available_semaphore);
	if (!swapchain_image.has_value()) {
		_request_resize();
		return;
	}

	vk::fence_reset(context, _get_current_frame().render_fence);

	const Vec2u swapchain_extent = vk::swapchain_get_extent(swapchain);
	const Vec3u draw_image_extent = vk::image_get_size(draw_image);

	// set render scale
	const float render_scale = get_settings().render_scale;
	draw_extent = Vec2u(
			std::min(swapchain_extent.x, draw_image_extent.x) * render_scale,
			std::min(swapchain_extent.y, draw_image_extent.y) * render_scale);

	CommandBuffer cmd = _get_current_frame().command_buffer;

	vk::command_reset(cmd);

	vk::command_begin(cmd);
	{
		vk::command_transition_image(
				cmd, draw_image, IMAGE_LAYOUT_UNDEFINED, IMAGE_LAYOUT_GENERAL);

		vk::command_clear_color(cmd, draw_image, { 0.1f, 0.4f, 0.7f, 1.0f });

		vk::command_transition_image(cmd, draw_image, IMAGE_LAYOUT_GENERAL,
				IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		vk::command_transition_image(cmd, swapchain_image.value(),
				IMAGE_LAYOUT_UNDEFINED, IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		vk::command_copy_image_to_image(cmd, draw_image,
				swapchain_image.value(), draw_extent, swapchain_extent);

		vk::command_transition_image(cmd, swapchain_image.value(),
				IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, IMAGE_LAYOUT_PRESENT_SRC);
	}
	vk::command_end(cmd);

	vk::queue_submit(backend->get_command_queue(QUEUE_TYPE_GRAPHICS), cmd,
			_get_current_frame().render_fence,
			_get_current_frame().image_available_semaphore,
			_get_current_frame().render_finished_semaphore);

	if (!vk::queue_present(context,
				backend->get_command_queue(QUEUE_TYPE_PRESENT), swapchain,
				_get_current_frame().render_finished_semaphore)) {
		_request_resize();
	}

	frame_number++;
}

SceneGraph& Renderer::get_scene_graph() { return scene_graph; }

RendererSettings& Renderer::get_settings() { return settings; }

RendererStats& Renderer::get_stats() { return stats; }

GraphicsAPI Renderer::get_graphics_api() { return s_api; }

void Renderer::_request_resize() {
	Application::get_instance()->enqueue_main_thread([&]() {
		vk::swapchain_resize(context,
				backend->get_command_queue(QUEUE_TYPE_PRESENT), swapchain,
				window->get_size());
	});
}

void Renderer::_destroy_scene_graph() {
	scene_graph.traverse([](Node* node) {
		switch (node->get_type()) {
			case NodeType::NONE: {
				break;
			}
			case NodeType::GEOMETRY: {
				break;
			}
			case NodeType::COMPUTE: {
				break;
			}
			case NodeType::CAMERA: {
				break;
			}
			case NodeType::LIGHT: {
				break;
			}
		}

		return false;
	});
}
