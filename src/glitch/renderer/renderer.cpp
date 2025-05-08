#include "glitch/renderer/renderer.h"

#include "glitch/core/application.h"
#include "glitch/renderer/types.h"

#include "glitch/platform/vulkan/vk_backend.h"

#include <imgui.h>

[[nodiscard]] GraphicsAPI find_proper_api() noexcept {
	return GRAPHICS_API_VULKAN;
}

static GraphicsAPI s_api;

Renderer* Renderer::s_instance = nullptr;

Renderer::Renderer(Ref<Window> p_window) : window(p_window) {
	GL_ASSERT(
			s_instance == nullptr, "Only one instance of renderer can exists!");
	s_instance = this;

	s_api = find_proper_api();
	switch (s_api) {
		case GRAPHICS_API_VULKAN:
			backend = create_ref<VulkanRenderBackend>();
			break;
		default:
			GL_ASSERT(false, "Selected graphics API not implemented.");
			break;
	}

	backend->init(p_window);

	graphics_queue = backend->queue_get(QUEUE_TYPE_GRAPHICS);
	// TODO: this is not being used right now
	present_queue = backend->queue_get(QUEUE_TYPE_PRESENT);

	swapchain = backend->swapchain_create();
	// initialize
	backend->swapchain_resize(present_queue, swapchain, p_window->get_size());

	draw_image = backend->image_create(draw_image_format, p_window->get_size(),
			nullptr,
			IMAGE_USAGE_TRANSFER_SRC_BIT | IMAGE_USAGE_TRANSFER_DST_BIT |
					IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

	depth_image =
			backend->image_create(depth_image_format, p_window->get_size(),
					nullptr, IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

	for (uint8_t i = 0; i < SWAPCHAIN_BUFFER_SIZE; i++) {
		FrameData& frame_data = frames[i];

		frame_data.command_pool = backend->command_pool_create(graphics_queue);
		frame_data.command_buffer =
				backend->command_pool_allocate(frame_data.command_pool);

		frame_data.image_available_semaphore = backend->semaphore_create();
		frame_data.render_finished_semaphore = backend->semaphore_create();

		frame_data.render_fence = backend->fence_create();
	}

	// initialize imgui context
	_imgui_init();
}

Renderer::~Renderer() {
	backend->device_wait();

	// destroy geometry pipeline resources
	backend->image_free(draw_image);
	backend->image_free(depth_image);

	// destroy per-frame data
	for (uint8_t i = 0; i < SWAPCHAIN_BUFFER_SIZE; i++) {
		FrameData& frame_data = frames[i];

		backend->command_pool_free(frame_data.command_pool);

		backend->semaphore_free(frame_data.image_available_semaphore);
		backend->semaphore_free(frame_data.render_finished_semaphore);

		backend->fence_free(frame_data.render_fence);
	}

	// swapchain cleanup
	backend->swapchain_free(swapchain);

	backend->shutdown();
}

CommandBuffer Renderer::begin_render() {
	_reset_stats();

	backend->fence_wait(_get_current_frame().render_fence);

	Optional<Image> swapchain_image = backend->swapchain_acquire_image(
			swapchain, _get_current_frame().image_available_semaphore);
	if (!swapchain_image) {
		_request_resize();
		return nullptr;
	}

	current_swapchain_image = *swapchain_image;

	backend->fence_reset(_get_current_frame().render_fence);

	const Vec2u swapchain_extent = backend->swapchain_get_extent(swapchain);
	const Vec3u draw_image_extent = backend->image_get_size(draw_image);

	// set render scale
	draw_extent = {
		std::min(swapchain_extent.x, draw_image_extent.x),
		std::min(swapchain_extent.y, draw_image_extent.y),
	};

	CommandBuffer cmd = _get_current_frame().command_buffer;

	backend->command_reset(cmd);

	backend->command_begin(cmd);

	backend->command_transition_image(
			cmd, draw_image, IMAGE_LAYOUT_UNDEFINED, IMAGE_LAYOUT_GENERAL);

	backend->command_clear_color(cmd, draw_image, { 0.1f, 0.1f, 0.1f, 1.0f });

	backend->command_transition_image(cmd, draw_image, IMAGE_LAYOUT_GENERAL,
			IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	backend->command_transition_image(cmd, depth_image, IMAGE_LAYOUT_UNDEFINED,
			IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

	// dynamic state
	backend->command_set_viewport(cmd, draw_extent);
	backend->command_set_scissor(cmd, draw_extent);

	return cmd;
}

void Renderer::end_render() {
	CommandBuffer cmd = _get_current_frame().command_buffer;

	backend->command_transition_image(cmd, draw_image,
			IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	backend->command_transition_image(cmd, current_swapchain_image,
			IMAGE_LAYOUT_UNDEFINED, IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	backend->command_copy_image_to_image(cmd, draw_image,
			current_swapchain_image, draw_extent,
			backend->swapchain_get_extent(swapchain));

	if (imgui_being_used) {
		backend->command_transition_image(cmd, current_swapchain_image,
				IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		_imgui_pass(cmd, current_swapchain_image);
	}

	backend->command_transition_image(cmd, current_swapchain_image,
			imgui_being_used ? IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
							 : IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			IMAGE_LAYOUT_PRESENT_SRC);

	backend->command_end(cmd);

	backend->queue_submit(graphics_queue, cmd,
			_get_current_frame().render_fence,
			_get_current_frame().image_available_semaphore,
			_get_current_frame().render_finished_semaphore);

	if (!backend->queue_present(graphics_queue, swapchain,
				_get_current_frame().render_finished_semaphore)) {
		_request_resize();
	}

	// reset the state
	imgui_being_used = false;
	frame_number++;

	// NOTE: maybe not?
	current_swapchain_image = nullptr;
}

void Renderer::wait_for_device() { backend->device_wait(); }

void Renderer::imgui_begin() {
	imgui_being_used = true;

	backend->imgui_new_frame_for_platform();

	ImGui::NewFrame();
}

void Renderer::imgui_end() {
	ImGui::Render();
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}

void Renderer::_imgui_pass(CommandBuffer p_cmd, Image p_target_image) {
	backend->command_begin_rendering(
			p_cmd, backend->swapchain_get_extent(swapchain), p_target_image);
	{ backend->imgui_render_for_platform(p_cmd); }
	backend->command_end_rendering(p_cmd);
}

void Renderer::_imgui_init() {
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	(void)io;

	io.ConfigFlags |=
			ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;
	io.IniFilename = ".glitch/imgui.ini";

	io.Fonts->Clear();

	backend->imgui_init_for_platform(window->get_native_window());
}

void Renderer::_request_resize() {
	Application::get_instance()->enqueue_main_thread([&]() {
		backend->swapchain_resize(present_queue, swapchain, window->get_size());
	});
}

void Renderer::_reset_stats() { memset(&stats, 0, sizeof(RenderStats)); }
