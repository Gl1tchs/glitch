#include "glitch/renderer/render_device.h"

#include "glitch/platform/vulkan/vk_backend.h"
#include "glitch/renderer/render_pass_builder.h"
#include "glitch/renderer/types.h"

#include <imgui.h>

[[nodiscard]] GraphicsAPI find_proper_api() noexcept {
	return GRAPHICS_API_VULKAN;
}

static GraphicsAPI s_api;

RenderDevice* RenderDevice::s_instance = nullptr;

RenderDevice::RenderDevice(Ref<Window> p_window) : window(p_window) {
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
	present_queue = backend->queue_get(QUEUE_TYPE_PRESENT);

	swapchain = backend->swapchain_create();
	color_attachment_format = backend->swapchain_get_format(swapchain);

	render_pass =
			RenderPassBuilder()
					.add_color_attachment(color_attachment_format)
					.add_depth_attachment(depth_image_format)
					.add_subpass({
							{
									{ 0, SUBPASS_ATTACHMENT_COLOR },
									{ 1, SUBPASS_ATTACHMENT_DEPTH_STENCIL },
							},
					})
					.build();

	// initialize swapchain, framebuffers and depth images
	_request_resize();

	for (size_t i = 0; i < SWAPCHAIN_BUFFER_SIZE; i++) {
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

RenderDevice::~RenderDevice() {
	backend->device_wait();

	for (auto& fb : swapchain_frame_buffers) {
		backend->frame_buffer_destroy(fb);
	}
	backend->render_pass_destroy(render_pass);

	// destroy geometry pipeline resources
	backend->image_free(depth_image);

	// destroy per-frame data
	for (auto& frame_data : frames) {
		backend->command_pool_free(frame_data.command_pool);

		backend->semaphore_free(frame_data.image_available_semaphore);
		backend->semaphore_free(frame_data.render_finished_semaphore);

		backend->fence_free(frame_data.render_fence);
	}

	// swapchain cleanup
	backend->swapchain_free(swapchain);

	backend->shutdown();
}

CommandBuffer RenderDevice::begin_render() {
	GL_PROFILE_SCOPE;

	_reset_stats();

	backend->fence_wait(_get_current_frame().render_fence);

	Optional<Image> swapchain_image = backend->swapchain_acquire_image(
			swapchain, _get_current_frame().image_available_semaphore,
			&image_index);
	if (!swapchain_image) {
		_request_resize();
		return nullptr;
	}

	current_swapchain_image = *swapchain_image;

	backend->fence_reset(_get_current_frame().render_fence);

	glm::uvec2 draw_extent = get_draw_extent();

	CommandBuffer cmd = _get_current_frame().command_buffer;

	backend->command_reset(cmd);

	backend->command_begin(cmd);

	backend->command_transition_image(cmd, current_swapchain_image,
			IMAGE_LAYOUT_UNDEFINED, IMAGE_LAYOUT_GENERAL);

	backend->command_clear_color(
			cmd, current_swapchain_image, { 0.1f, 0.1f, 0.1f, 1.0f });

	backend->command_transition_image(cmd, current_swapchain_image,
			IMAGE_LAYOUT_GENERAL, IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	backend->command_transition_image(cmd, depth_image, IMAGE_LAYOUT_UNDEFINED,
			IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

	// dynamic state
	backend->command_set_viewport(cmd, draw_extent);
	backend->command_set_scissor(cmd, draw_extent);

	return cmd;
}

void RenderDevice::end_render() {
	GL_PROFILE_SCOPE;

	if (!current_swapchain_image) {
		GL_LOG_FATAL(
				"RenderDevice::end_render: There is no image to render to!");
	}

	CommandBuffer cmd = _get_current_frame().command_buffer;

	if (imgui_being_used) {
		_imgui_pass(cmd, current_swapchain_image);
	}

	backend->command_transition_image(cmd, current_swapchain_image,
			IMAGE_LAYOUT_UNDEFINED, IMAGE_LAYOUT_PRESENT_SRC);

	backend->command_end(cmd);

	backend->queue_submit(graphics_queue, cmd,
			_get_current_frame().render_fence,
			_get_current_frame().image_available_semaphore,
			_get_current_frame().render_finished_semaphore);

	if (!backend->queue_present(present_queue, swapchain,
				_get_current_frame().render_finished_semaphore)) {
		_request_resize();
	}

	// reset the state
	imgui_being_used = false;
	frame_number++;

	// NOTE: maybe not?
	current_swapchain_image = nullptr;
}

void RenderDevice::wait_for_device() { backend->device_wait(); }

void RenderDevice::imgui_begin() {
	GL_PROFILE_SCOPE;

	imgui_being_used = true;

	backend->imgui_new_frame_for_platform();

	ImGui::NewFrame();
}

void RenderDevice::imgui_end() {
	GL_PROFILE_SCOPE;

	ImGui::Render();
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}

glm::uvec2 RenderDevice::get_draw_extent() const {
	const glm::uvec2 swapchain_extent =
			backend->swapchain_get_extent(swapchain);

	if (!current_swapchain_image) {
		return swapchain_extent;
	}

	const glm::uvec3 draw_image_extent =
			backend->image_get_size(current_swapchain_image);

	return {
		std::min(swapchain_extent.x, draw_image_extent.x),
		std::min(swapchain_extent.y, draw_image_extent.y),
	};
}

std::vector<FrameBuffer> RenderDevice::get_swapchain_framebuffers(
		RenderPass p_render_pass) {
	std::vector<FrameBuffer> framebuffers;
	for (const auto& attachment : backend->swapchain_get_images(swapchain)) {
		std::vector<Image> fb_attachments = {
			attachment,
			depth_image,
		};

		framebuffers.push_back(backend->frame_buffer_create(
				p_render_pass, fb_attachments, get_draw_extent()));
	}

	return framebuffers;
}

RenderPass RenderDevice::get_render_pass() { return render_pass; }

FrameBuffer RenderDevice::get_current_frame_buffer() {
	return swapchain_frame_buffers[image_index];
}

uint32_t RenderDevice::get_current_image_index() const { return image_index; }

Swapchain RenderDevice::get_swapchain() { return swapchain; }

Image RenderDevice::get_draw_image() { return current_swapchain_image; }

Image RenderDevice::get_depth_image() { return depth_image; }

RenderStats& RenderDevice::get_stats() { return stats; }

DataFormat RenderDevice::get_color_attachment_format() {
	return s_instance->color_attachment_format;
}

DataFormat RenderDevice::get_depth_attachment_format() {
	return s_instance->depth_image_format;
}

Ref<RenderBackend> RenderDevice::get_backend() { return s_instance->backend; }

void RenderDevice::_imgui_pass(CommandBuffer p_cmd, Image p_target_image) {
	GL_PROFILE_SCOPE;

	backend->command_begin_rendering(
			p_cmd, backend->swapchain_get_extent(swapchain), p_target_image);
	{
		backend->imgui_render_for_platform(p_cmd);
	}
	backend->command_end_rendering(p_cmd);
}

void RenderDevice::_imgui_init() {
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	(void)io;

	io.ConfigFlags |=
			ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;
	io.IniFilename = ".glitch/imgui.ini";

	io.Fonts->Clear();

	backend->imgui_init_for_platform(window->get_native_window());

	// ImGui style changes
	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4* colors = style.Colors;

	// Make window backgrounds semi-transparent
	const ImVec4 bg_color = ImVec4(0.1, 0.1, 0.1, 0.5);
	colors[ImGuiCol_WindowBg] = bg_color;
	colors[ImGuiCol_ChildBg] = bg_color;
	colors[ImGuiCol_TitleBg] = bg_color;
}

void RenderDevice::_request_resize() {
	GL_PROFILE_SCOPE_N("RenderDevice::Swapchain Resize");

	glm::uvec2 new_size = window->get_size();

	backend->swapchain_resize(graphics_queue, swapchain, new_size);

	// Resize depth image as well
	if (depth_image) {
		backend->image_free(depth_image);
	}

	depth_image = backend->image_create(depth_image_format, new_size, nullptr,
			IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

	for (auto& fb : swapchain_frame_buffers) {
		backend->frame_buffer_destroy(fb);
	}
	swapchain_frame_buffers.clear();

	swapchain_frame_buffers = get_swapchain_framebuffers(render_pass);
}

void RenderDevice::_reset_stats() { memset(&stats, 0, sizeof(RenderStats)); }
