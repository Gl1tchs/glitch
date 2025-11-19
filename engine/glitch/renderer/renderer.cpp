#include "glitch/renderer/renderer.h"

#include "glitch/platform/vulkan/vk_backend.h"
#include "glitch/renderer/graphics_pass.h"
#include "glitch/renderer/mesh.h"
#include "glitch/renderer/types.h"

#include <imgui.h>

namespace gl {

static Renderer* s_instance = nullptr;
static std::shared_ptr<RenderBackend> s_backend = nullptr;

void FrameData::init(CommandQueue p_queue) {
	std::shared_ptr<RenderBackend> backend = Renderer::get_backend();

	command_pool = backend->command_pool_create(p_queue);
	command_buffer = backend->command_pool_allocate(command_pool);

	image_available_semaphore = backend->semaphore_create();
	render_finished_semaphore = backend->semaphore_create();

	render_fence = backend->fence_create();
}

void FrameData::destroy() {
	std::shared_ptr<RenderBackend> backend = Renderer::get_backend();

	backend->command_pool_free(command_pool);

	backend->semaphore_free(image_available_semaphore);
	backend->semaphore_free(render_finished_semaphore);

	backend->fence_free(render_fence);
}

Renderer::Renderer(std::shared_ptr<Window> p_window, RendererSettings p_settings) :
		window(p_window), settings(p_settings) {
	GL_ASSERT(s_instance == nullptr, "Only one instance of renderer can exists!");
	s_instance = this;

	// We do not support other render backends for now.
	s_backend = std::make_shared<VulkanRenderBackend>();
	s_backend->init(window);

	default_sampler = s_backend->sampler_create();

	graphics_queue = s_backend->queue_get(QueueType::GRAPHICS);
	present_queue = s_backend->queue_get(QueueType::PRESENT);

	// initialize swapchain
	const glm::uvec2 window_px = window->get_size();
	swapchain = s_backend->swapchain_create();
	s_backend->swapchain_resize(graphics_queue, swapchain, window_px, p_settings.vsync);

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
	s_backend->device_wait();

	// destroy image and renderpass resources
	s_backend->image_free(final_image);
	for (auto& [name, render_image] : renderpass_images) {
		s_backend->image_free(render_image.image);
	}

	// Explicitly delete graphics passes
	for (auto& [pass, _] : graphics_passes) {
		pass.reset();
	}

	// destroy per-frame data
	for (auto& frame_data : frames) {
		frame_data.destroy();
	}

	// swapchain cleanup
	s_backend->swapchain_free(swapchain);

	s_backend->sampler_free(default_sampler);

	s_backend->shutdown();
}

CommandBuffer Renderer::begin_render() {
	GL_PROFILE_SCOPE;

	_reset_stats();

	s_backend->fence_wait(_get_current_frame().render_fence);

	const Result<Image, SwapchainAcquireError> swapchain_image = s_backend->swapchain_acquire_image(
			swapchain, _get_current_frame().image_available_semaphore, &image_index);
	if (!swapchain_image) {
		switch (swapchain_image.get_error()) {
			case SwapchainAcquireError::OUT_OF_DATE:
				_request_resize();
				return nullptr;
			case SwapchainAcquireError::ERROR:
				GL_LOG_FATAL("[Renderer::begin_render] Failed to acquire swapchain image.");
				return nullptr;
		}
	}

	current_swapchain_image = *swapchain_image;

	s_backend->fence_reset(_get_current_frame().render_fence);

	const glm::uvec3 draw_image_extent = glm::uvec3(get_resolution_extent(), 1.0);

	CommandBuffer cmd = _get_current_frame().command_buffer;

	s_backend->command_reset(cmd);
	s_backend->command_begin(cmd);

	// Transition renderpass attachments
	for (const auto& [id, render_image] : renderpass_images) {
		if (render_image.is_depth_attachment) {
			s_backend->command_transition_image(cmd, render_image.image, ImageLayout::UNDEFINED,
					ImageLayout::DEPTH_ATTACHMENT_OPTIMAL);
		} else {
			s_backend->command_transition_image(cmd, render_image.image, ImageLayout::UNDEFINED,
					ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
		}
	}

	// Just so we can use msaa
	s_backend->command_transition_image(cmd, current_swapchain_image, ImageLayout::UNDEFINED,
			ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
	s_backend->command_transition_image(
			cmd, final_image, ImageLayout::UNDEFINED, ImageLayout::COLOR_ATTACHMENT_OPTIMAL);

	// dynamic state
	s_backend->command_set_viewport(cmd, draw_image_extent);
	s_backend->command_set_scissor(cmd, draw_image_extent);

	return cmd;
}

void Renderer::end_render() {
	GL_PROFILE_SCOPE;

	if (should_present_to_swapchain && !current_swapchain_image) {
		GL_LOG_FATAL("[Renderer::end_render] There is no image to render to!");
	}

	CommandBuffer cmd = _get_current_frame().command_buffer;

	const bool msaa_used = msaa_samples != 1;

	// Get final image to copy
	Image final_color_attachment = GL_NULL_HANDLE;
	if (swapchain_target_image_id.empty()) {
		for (const auto& [id, render_image] : renderpass_images) {
			if (render_image.is_depth_attachment) {
				continue;
			}

			final_color_attachment = render_image.image;
		}
	} else {
		const auto final_color_attachment_it = renderpass_images.find(swapchain_target_image_id);
		if (final_color_attachment_it != renderpass_images.end()) {
			final_color_attachment = final_color_attachment_it->second.image;
		}
	}

	// TODO!
	if (!final_color_attachment) {
		GL_LOG_ERROR("[Renderer::end_render] Unable to find color attachment to render");
		return;
	}

	const Image render_target = should_present_to_swapchain ? current_swapchain_image : final_image;

	// Copy color image to final image if there isn't got a resolver
	if (!msaa_used) {
		s_backend->command_transition_image(cmd, final_color_attachment,
				ImageLayout::COLOR_ATTACHMENT_OPTIMAL, ImageLayout::TRANSFER_SRC_OPTIMAL);
		s_backend->command_transition_image(cmd, render_target,
				ImageLayout::COLOR_ATTACHMENT_OPTIMAL, ImageLayout::TRANSFER_DST_OPTIMAL);

		s_backend->command_copy_image_to_image(cmd, final_color_attachment, render_target,
				s_backend->image_get_size(final_color_attachment),
				s_backend->image_get_size(render_target));
	}

	// Transition into shader read only to use as a viewport image
	// Do this before imgui, if final_image being used as an imgui image
	// image layout must be set to SHADER_READ_ONLY_OPTIMAL
	if (!should_present_to_swapchain) {
		s_backend->command_transition_image(cmd, final_image,
				!msaa_used ? ImageLayout::TRANSFER_DST_OPTIMAL
						   : ImageLayout::COLOR_ATTACHMENT_OPTIMAL,
				ImageLayout::SHADER_READ_ONLY_OPTIMAL);
	}

	if (imgui_being_used) {
		if (should_present_to_swapchain && !msaa_used) {
			s_backend->command_transition_image(cmd, current_swapchain_image,
					ImageLayout::TRANSFER_DST_OPTIMAL, ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
		}

		_imgui_pass(cmd, current_swapchain_image);
	}

	s_backend->command_transition_image(cmd, current_swapchain_image,
			(should_present_to_swapchain && !msaa_used && !imgui_being_used)
					? ImageLayout::TRANSFER_DST_OPTIMAL
					: ImageLayout::COLOR_ATTACHMENT_OPTIMAL,
			ImageLayout::PRESENT_SRC);

	s_backend->command_end(cmd);

	s_backend->queue_submit(graphics_queue, cmd, _get_current_frame().render_fence,
			_get_current_frame().image_available_semaphore,
			_get_current_frame().render_finished_semaphore);

	if (!s_backend->queue_present(
				present_queue, swapchain, _get_current_frame().render_finished_semaphore)) {
		_request_resize();
	}

	// reset the state
	imgui_being_used = false;
	current_swapchain_image = nullptr;
	frame_number++;
}

void Renderer::add_pass(std::shared_ptr<GraphicsPass> p_pass, int p_priority) {
	p_pass->setup(*this);

	graphics_passes.push_back(std::make_pair(p_pass, p_priority));
}

void Renderer::execute(CommandBuffer p_cmd) {
	// Sort based on priority
	std::sort(graphics_passes.begin(), graphics_passes.end(),
			[](const auto& lhs, const auto& rhs) -> bool { return lhs.second < rhs.second; });

	for (const auto& [pass, _prior] : graphics_passes) {
		if (!pass->is_active()) {
			continue;
		}

		pass->execute(p_cmd, *this);
	}
}

void Renderer::begin_rendering(CommandBuffer p_cmd, Image p_color_attachment,
		Image p_depth_attachment, std::optional<Color> p_clear_color) {
	RenderingAttachment color_attachment = {};
	color_attachment.image = p_color_attachment;
	color_attachment.layout = ImageLayout::COLOR_ATTACHMENT_OPTIMAL;

	if (p_clear_color) {
		color_attachment.load_op = AttachmentLoadOp::CLEAR;
		color_attachment.clear_color = *p_clear_color;
	} else {
		color_attachment.load_op = AttachmentLoadOp::LOAD;
	}

	if (msaa_samples != 1) {
		color_attachment.resolve_mode = RESOLVE_MODE_AVERAGE_BIT;
		color_attachment.resolve_image =
				should_present_to_swapchain ? current_swapchain_image : final_image;
		color_attachment.resolve_layout = ImageLayout::COLOR_ATTACHMENT_OPTIMAL;
	}

	s_backend->command_begin_rendering(p_cmd, s_backend->image_get_size(p_color_attachment),
			color_attachment, p_depth_attachment);
}

void Renderer::end_rendering(CommandBuffer p_cmd) { s_backend->command_end_rendering(p_cmd); }

Result<Image, Renderer::ImageCreateError> Renderer::create_render_image(
		const std::string& p_name, DataFormat p_format, BitField<ImageUsageBits> p_usage) {
	if (renderpass_images.find(p_name) != renderpass_images.end()) {
		return make_err<Image>(ImageCreateError::ID_EXISTS);
	}

	if (!p_usage.has_flag(IMAGE_USAGE_TRANSFER_SRC_BIT)) {
		p_usage.set_flag(IMAGE_USAGE_TRANSFER_SRC_BIT);
	}
	if (!p_usage.has_flag(IMAGE_USAGE_TRANSFER_DST_BIT)) {
		p_usage.set_flag(IMAGE_USAGE_TRANSFER_DST_BIT);
	}

	const bool is_depth_format = p_format == DataFormat::D16_UNORM ||
			p_format == DataFormat::D16_UNORM_S8_UINT ||
			p_format == DataFormat::D24_UNORM_S8_UINT || p_format == DataFormat::D32_SFLOAT;

	RenderImage render_image;
	render_image.image = s_backend->image_create(
			p_format, get_resolution_extent(), nullptr, p_usage, false, msaa_samples);
	render_image.format = p_format;
	render_image.usage = p_usage;
	render_image.is_depth_attachment = is_depth_format;

	// bookkeep
	renderpass_images[p_name] = render_image;

	return render_image.image;
}

std::optional<Image> Renderer::get_render_image(const std::string& p_name) {
	const auto it = renderpass_images.find(p_name);
	if (it == renderpass_images.end()) {
		return {};
	}

	return it->second.image;
}

void Renderer::set_swapchain_target(const std::string& p_name) {
	swapchain_target_image_id = p_name;
}

void Renderer::wait_for_device() { s_backend->device_wait(); }

void Renderer::imgui_begin() {
	GL_PROFILE_SCOPE;

	imgui_being_used = true;

	s_backend->imgui_new_frame_for_platform();

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

float Renderer::get_resolution_scale() const { return settings.resolution_scale; }

void Renderer::set_resolution_scale(float p_scale) {
	if (fabs(p_scale - settings.resolution_scale) > 0.001f) {
		settings.resolution_scale = p_scale;

		_request_resize();
	}
}

bool Renderer::get_vsync_enabled() const { return settings.vsync; }

void Renderer::set_vsync(bool p_vsync) {
	// avoid unnecessary resize
	if (settings.vsync != p_vsync) {
		settings.vsync = p_vsync;

		_request_resize();
	}
}

uint32_t Renderer::get_msaa_samples() const { return msaa_samples; }

void Renderer::set_msaa_samples(uint32_t p_samples) {
	const uint32_t max_sample_count = s_backend->get_max_msaa_samples();

	if ((p_samples != 1 && p_samples % 2 != 0) || p_samples > max_sample_count) {
		GL_LOG_ERROR("[Renderer::set_msaa_samples] Invalid MSAA sample count: {}. Must be 1 or "
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

glm::uvec2 Renderer::get_resolution_extent() const {
	const glm::uvec2 swapchain_size = s_backend->swapchain_get_extent(swapchain);
	return {
		std::max(1u, uint32_t(swapchain_size.x * settings.resolution_scale)),
		std::max(1u, uint32_t(swapchain_size.y * settings.resolution_scale)),
	};
}

void* Renderer::get_final_image_descriptor() const { return final_image_descriptor; }

glm::uvec2 Renderer::get_final_image_size() const { return s_backend->image_get_size(final_image); }

RenderStats& Renderer::get_stats() { return stats; }

std::shared_ptr<RenderBackend> Renderer::get_backend() { return s_backend; }

void Renderer::_imgui_pass(CommandBuffer p_cmd, Image p_target_image) {
	GL_PROFILE_SCOPE;

	RenderingAttachment color_attachment = {};
	color_attachment.image = p_target_image;
	color_attachment.layout = ImageLayout::COLOR_ATTACHMENT_OPTIMAL;

	s_backend->command_begin_rendering(
			p_cmd, s_backend->image_get_size(p_target_image), color_attachment);

	s_backend->imgui_render_for_platform(p_cmd);

	s_backend->command_end_rendering(p_cmd);
}

void Renderer::_imgui_init() {
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	(void)io;

	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;
	io.IniFilename = ".glitch/imgui.ini";

	io.Fonts->Clear();

	s_backend->imgui_init_for_platform(
			window->get_native_window(), s_backend->swapchain_get_format(swapchain));

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
	s_backend->swapchain_resize(graphics_queue, swapchain, window_px, settings.vsync);

	const glm::uvec2 new_size = get_resolution_extent();

	// Resize depth, color and final image
	if (final_image) {
		s_backend->image_free(final_image);
	}
	final_image =
			s_backend->image_create(s_backend->swapchain_get_format(swapchain), new_size, nullptr,
					IMAGE_USAGE_COLOR_ATTACHMENT_BIT | IMAGE_USAGE_SAMPLED_BIT |
							IMAGE_USAGE_TRANSFER_DST_BIT);

	if (final_image_descriptor) {
		s_backend->imgui_image_free(final_image_descriptor);
	}
	final_image_descriptor = s_backend->imgui_image_upload(final_image, default_sampler);

	// Recreate renderpass attachments
	for (auto& [id, render_image] : renderpass_images) {
		if (render_image.image) {
			s_backend->image_free(render_image.image);
		}

		render_image.image = s_backend->image_create(
				render_image.format, new_size, nullptr, render_image.usage, false, msaa_samples);
	}
}

void Renderer::_reset_stats() { memset(&stats, 0, sizeof(RenderStats)); }

} //namespace gl
