#include "glitch/renderer/renderer.h"

#include "glitch/platform/vulkan/vk_backend.h"
#include "glitch/renderer/types.h"

#include <imgui.h>

namespace gl {

[[nodiscard]] GraphicsAPI get_proper_render_backend() noexcept {
	return GraphicsAPI::VULKAN;
}

static GraphicsAPI s_api;

Renderer* Renderer::s_instance = nullptr;

void FrameData::init(CommandQueue p_queue) {
	Ref<RenderBackend> backend = Renderer::get_backend();

	command_pool = backend->command_pool_create(p_queue);
	command_buffer = backend->command_pool_allocate(command_pool);

	image_available_semaphore = backend->semaphore_create();
	render_finished_semaphore = backend->semaphore_create();

	render_fence = backend->fence_create();
}

void FrameData::destroy() {
	Ref<RenderBackend> backend = Renderer::get_backend();

	backend->command_pool_free(command_pool);

	backend->semaphore_free(image_available_semaphore);
	backend->semaphore_free(render_finished_semaphore);

	backend->fence_free(render_fence);
}

Renderer::Renderer(Ref<Window> p_window) : window(p_window) {
	GL_ASSERT(
			s_instance == nullptr, "Only one instance of renderer can exists!");
	s_instance = this;

	s_api = get_proper_render_backend();
	switch (s_api) {
		case GraphicsAPI::VULKAN:
			backend = create_ref<VulkanRenderBackend>();
			break;
		default:
			GL_ASSERT(false, "Selected graphics API not implemented.");
			break;
	}

	backend->init(window);

	default_sampler = backend->sampler_create();

	graphics_queue = backend->queue_get(QueueType::GRAPHICS);
	present_queue = backend->queue_get(QueueType::PRESENT);

	// initialize swapchain
	const glm::uvec2 window_px = window->get_size();
	swapchain = backend->swapchain_create();
	backend->swapchain_resize(graphics_queue, swapchain, window_px);

	// initialize imgui
	_imgui_init();

	// initialize framebuffers and render images
	_request_resize();

	for (size_t i = 0; i < SWAPCHAIN_BUFFER_SIZE; i++) {
		FrameData& frame_data = frames[i];
		frame_data.init(graphics_queue);
	}
}

Renderer::~Renderer() {
	backend->device_wait();

	// destroy geometry pipeline resources
	backend->image_free(final_image);
	backend->image_free(color_image);
	backend->image_free(depth_image);

	// destroy per-frame data
	for (auto& frame_data : frames) {
		frame_data.destroy();
	}

	// swapchain cleanup
	backend->swapchain_free(swapchain);

	backend->sampler_free(default_sampler);

	backend->shutdown();
}

CommandBuffer Renderer::begin_render() {
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

	const glm::uvec3 draw_image_extent = backend->image_get_size(color_image);

	CommandBuffer cmd = _get_current_frame().command_buffer;

	backend->command_reset(cmd);

	backend->command_begin(cmd);

	backend->command_transition_image(cmd, color_image, ImageLayout::UNDEFINED,
			ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
	backend->command_transition_image(cmd, depth_image, ImageLayout::UNDEFINED,
			ImageLayout::DEPTH_ATTACHMENT_OPTIMAL);

	// Just so we can use msaa
	backend->command_transition_image(cmd, current_swapchain_image,
			ImageLayout::UNDEFINED, ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
	backend->command_transition_image(cmd, final_image, ImageLayout::UNDEFINED,
			ImageLayout::COLOR_ATTACHMENT_OPTIMAL);

	// dynamic state
	backend->command_set_viewport(cmd, draw_image_extent);
	backend->command_set_scissor(cmd, draw_image_extent);

	return cmd;
}

void Renderer::end_render() {
	GL_PROFILE_SCOPE;

	if (should_present_to_swapchain && !current_swapchain_image) {
		GL_LOG_FATAL("Renderer::end_render: There is no image to render to!");
	}

	CommandBuffer cmd = _get_current_frame().command_buffer;

	const bool msaa_used = msaa_samples != 1;

	Image render_target =
			should_present_to_swapchain ? current_swapchain_image : final_image;

	// Copy color image to final image if there isn't got a resolver
	if (!msaa_used) {
		backend->command_transition_image(cmd, color_image,
				ImageLayout::COLOR_ATTACHMENT_OPTIMAL,
				ImageLayout::TRANSFER_SRC_OPTIMAL);
		backend->command_transition_image(cmd, render_target,
				ImageLayout::COLOR_ATTACHMENT_OPTIMAL,
				ImageLayout::TRANSFER_DST_OPTIMAL);

		backend->command_copy_image_to_image(cmd, color_image, render_target,
				backend->image_get_size(color_image),
				backend->image_get_size(render_target));
	}

	// Transition into shader read only to use as a viewport image
	// Do this before imgui, if final_image being used as an imgui image
	// image layout must be set to SHADER_READ_ONLY_OPTIMAL
	if (!should_present_to_swapchain) {
		backend->command_transition_image(cmd, final_image,
				!msaa_used ? ImageLayout::TRANSFER_DST_OPTIMAL
						   : ImageLayout::COLOR_ATTACHMENT_OPTIMAL,
				ImageLayout::SHADER_READ_ONLY_OPTIMAL);
	}

	if (imgui_being_used) {
		if (should_present_to_swapchain && !msaa_used) {
			backend->command_transition_image(cmd, current_swapchain_image,
					ImageLayout::TRANSFER_DST_OPTIMAL,
					ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
		}

		_imgui_pass(cmd, current_swapchain_image);
	}

	backend->command_transition_image(cmd, current_swapchain_image,
			(should_present_to_swapchain && !msaa_used && !imgui_being_used)
					? ImageLayout::TRANSFER_DST_OPTIMAL
					: ImageLayout::COLOR_ATTACHMENT_OPTIMAL,
			ImageLayout::PRESENT_SRC);

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
	current_swapchain_image = nullptr;
	frame_number++;
}

void Renderer::begin_rendering(CommandBuffer p_cmd) {
	RenderingAttachment color_attachment = {};
	color_attachment.image = color_image;
	color_attachment.layout = ImageLayout::COLOR_ATTACHMENT_OPTIMAL;
	color_attachment.load_op = AttachmentLoadOp::CLEAR;
	color_attachment.clear_color = settings.clear_color;

	if (msaa_samples != 1) {
		color_attachment.resolve_mode = RESOLVE_MODE_AVERAGE_BIT;
		color_attachment.resolve_image = should_present_to_swapchain
				? current_swapchain_image
				: final_image;
		color_attachment.resolve_layout = ImageLayout::COLOR_ATTACHMENT_OPTIMAL;
	}

	backend->command_begin_rendering(p_cmd,
			backend->image_get_size(color_image), color_attachment,
			depth_image);
}

void Renderer::end_rendering(CommandBuffer p_cmd) {
	backend->command_end_rendering(p_cmd);
}

void Renderer::wait_for_device() { backend->device_wait(); }

void Renderer::imgui_begin() {
	GL_PROFILE_SCOPE;

	imgui_being_used = true;

	backend->imgui_new_frame_for_platform();

	ImGui::NewFrame();
}

void Renderer::imgui_end() {
	GL_PROFILE_SCOPE;

	ImGui::Render();
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}

void Renderer::set_render_present_mode(bool p_present_to_swapchain) {
	should_present_to_swapchain = p_present_to_swapchain;
}

void Renderer::set_clear_color(Color p_color) {
	settings.clear_color = p_color;
}

void Renderer::set_resolution_scale(float p_scale) {
	if (fabs(p_scale - settings.resolution_scale) > 0.001f) {
		settings.resolution_scale = p_scale;

		_request_resize();
	}
}

uint32_t Renderer::get_msaa_samples() const { return msaa_samples; }

void Renderer::set_msaa_samples(uint32_t p_samples) {
	const uint32_t max_sample_count = backend->get_max_msaa_samples();

	if ((p_samples != 1 && p_samples % 2 != 0) ||
			p_samples > max_sample_count) {
		GL_LOG_ERROR("Invalid MSAA sample count: {}. Must be 1 or "
					 "power-of-two, and ≤ {}",
				p_samples, max_sample_count);
		msaa_samples = 1;
		return;
	}

	if (msaa_samples != p_samples) {
		msaa_samples = p_samples;

		_request_resize();
	}
}

Swapchain Renderer::get_swapchain() { return swapchain; }

void* Renderer::get_final_image_descriptor() const {
	return final_image_descriptor;
}

glm::uvec2 Renderer::get_final_image_size() const {
	return backend->image_get_size(final_image);
}

RenderStats& Renderer::get_stats() { return stats; }

DataFormat Renderer::get_color_attachment_format() {
	return s_instance->color_attachment_format;
}

DataFormat Renderer::get_depth_attachment_format() {
	return s_instance->depth_attachment_format;
}

Ref<RenderBackend> Renderer::get_backend() { return s_instance->backend; }

void Renderer::_imgui_pass(CommandBuffer p_cmd, Image p_target_image) {
	GL_PROFILE_SCOPE;

	RenderingAttachment color_attachment = {};
	color_attachment.image = p_target_image;
	color_attachment.layout = ImageLayout::COLOR_ATTACHMENT_OPTIMAL;

	backend->command_begin_rendering(
			p_cmd, backend->image_get_size(p_target_image), color_attachment);

	backend->imgui_render_for_platform(p_cmd);

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

	backend->imgui_init_for_platform(window->get_native_window(),
			backend->swapchain_get_format(swapchain));

	// ImGui style changes
	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4* colors = style.Colors;

	// Make window backgrounds semi-transparent
	const ImVec4 bg_color = ImVec4(0.1, 0.1, 0.1, 0.5);
	colors[ImGuiCol_WindowBg] = bg_color;
	colors[ImGuiCol_ChildBg] = bg_color;
	colors[ImGuiCol_TitleBg] = bg_color;
}

void Renderer::_request_resize() {
	GL_PROFILE_SCOPE_N("Renderer::Swapchain Resize");

	const glm::uvec2 window_px = window->get_size();
	backend->swapchain_resize(graphics_queue, swapchain, window_px);

	const glm::uvec2 new_size = {
		std::max(1u, uint32_t(window_px.x * settings.resolution_scale)),
		std::max(1u, uint32_t(window_px.y * settings.resolution_scale)),
	};

	// Resize depth, color and final image
	if (final_image) {
		backend->image_free(final_image);
	}
	final_image =
			backend->image_create(color_attachment_format, new_size, nullptr,
					IMAGE_USAGE_COLOR_ATTACHMENT_BIT | IMAGE_USAGE_SAMPLED_BIT |
							IMAGE_USAGE_TRANSFER_DST_BIT);

	if (final_image_descriptor) {
		backend->imgui_image_free(final_image_descriptor);
	}
	final_image_descriptor =
			backend->imgui_image_upload(final_image, default_sampler);

	if (color_image) {
		backend->image_free(color_image);
	}
	color_image = backend->image_create(color_attachment_format, new_size,
			nullptr,
			IMAGE_USAGE_COLOR_ATTACHMENT_BIT | IMAGE_USAGE_TRANSFER_SRC_BIT |
					IMAGE_USAGE_TRANSFER_DST_BIT,
			false, msaa_samples);

	if (depth_image) {
		backend->image_free(depth_image);
	}
	depth_image = backend->image_create(depth_attachment_format, new_size,
			nullptr, IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, false,
			msaa_samples);
}

void Renderer::_reset_stats() { memset(&stats, 0, sizeof(RenderStats)); }

} //namespace gl