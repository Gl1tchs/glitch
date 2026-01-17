#include "glitch/renderer/renderer.h"

#include "glitch/renderer/graphics_pass.h"

#include <glgpu/glgpu.h>

#include <imgui.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <format>
#include <memory>
#include <string>
#include <vector>

namespace gl {

static Renderer* s_instance = nullptr;
static std::shared_ptr<Device> s_device = nullptr;

void FrameData::init(Device* device, CommandQueue queue) {
	command_pool = device->command_pool_create(queue).value();
	command_buffer = device->command_pool_allocate(command_pool).value();

	image_available_semaphore = device->semaphore_create();
	render_finished_semaphore = device->semaphore_create();

	render_fence = device->fence_create();
}

void FrameData::destroy(Device* device) {
	device->command_pool_free(command_pool);

	device->semaphore_free(image_available_semaphore);
	device->semaphore_free(render_finished_semaphore);

	device->fence_free(render_fence);
}

Renderer::Renderer(std::shared_ptr<Window> window, RendererSettings settings) :
		_window(window), _settings(settings) {
	GL_ASSERT(s_instance == nullptr, "Only one instance of renderer can exists!");
	s_instance = this;

	// Create glgpu device
	DeviceCreateInfo info{
		.required_features = DEVICE_FEATURE_SWAPCHAIN_BIT | DEVICE_FEATURE_ENSURE_SURFACE_SUPPORT |
				DEVICE_FEATURE_VALIDATION_LAYERS,
	};

	// For now, we'll create device without window attachment
	// TODO: Handle window attachment properly for GLFW
	s_device = Device::create(info).value();

	_default_sampler = s_device->sampler_create({}).value();

	_graphics_queue = s_device->queue_get(QueueType::GRAPHICS).value();
	_present_queue = s_device->queue_get(QueueType::PRESENT).value();

	// initialize swapchain
	const auto window_size = _window->get_size();
	const Vec2u window_px = { window_size.x, window_size.y };
	_swapchain = s_device->swapchain_create().value();
	s_device->swapchain_resize(_graphics_queue, _swapchain, window_px, settings.vsync);

	// TODO: Re-implement imgui integration for glgpu
	// _imgui_init();

	// initialize framebuffers and render images
	_request_resize();

	for (size_t i = 0; i < SWAPCHAIN_BUFFER_SIZE; i++) {
		FrameData& frame_data = _frames[i];
		frame_data.init(s_device.get(), _graphics_queue);
	}
}

Renderer::~Renderer() {
	s_device->device_wait();

	// destroy image and renderpass resources
	s_device->image_free(_final_image);
	for (auto& [name, render_image] : _renderpass_images) {
		s_device->image_free(render_image.image);
	}

	// Explicitly delete graphics passes
	for (auto& [pass, _] : _graphics_passes) {
		pass.reset();
	}

	// destroy per-frame data
	for (auto& frame_data : _frames) {
		frame_data.destroy(s_device.get());
	}

	// swapchain cleanup
	s_device->swapchain_free(_swapchain);

	s_device->sampler_free(_default_sampler);
}

CommandBuffer Renderer::begin_render() {
	// GL_PROFILE_SCOPE;

	_reset_stats();

	s_device->fence_wait(_get_current_frame().render_fence);

	auto swapchain_result = s_device->swapchain_acquire_image(
			_swapchain, _get_current_frame().image_available_semaphore, &_image_index);
	if (!swapchain_result) {
		// For now, assume any error means resize needed
		_request_resize();
		return nullptr;
	}

	_current_swapchain_image = *swapchain_result;

	s_device->fence_reset(_get_current_frame().render_fence);

	const Vec3u draw_image_extent = Vec3u(get_resolution_extent(), 1);

	CommandBuffer cmd = _get_current_frame().command_buffer;

	s_device->command_reset(cmd);
	s_device->command_begin(cmd);

	// Transition renderpass attachments
	for (const auto& [id, render_image] : _renderpass_images) {
		if (render_image.is_depth_attachment) {
			s_device->command_transition_image(cmd, render_image.image, ImageLayout::UNDEFINED,
					ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		} else {
			s_device->command_transition_image(cmd, render_image.image, ImageLayout::UNDEFINED,
					ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
		}
	}

	// Just so we can use msaa
	s_device->command_transition_image(cmd, _current_swapchain_image, ImageLayout::UNDEFINED,
			ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
	s_device->command_transition_image(
			cmd, _final_image, ImageLayout::UNDEFINED, ImageLayout::COLOR_ATTACHMENT_OPTIMAL);

	// dynamic state
	s_device->command_set_viewport(cmd, draw_image_extent);
	s_device->command_set_scissor(cmd, draw_image_extent);

	return cmd;
}

void Renderer::end_render() {
	// GL_PROFILE_SCOPE;

	if (_should_present_to_swapchain && !_current_swapchain_image) {
		GL_LOG_FATAL("[Renderer::end_render] There is no image to render to!");
	}

	CommandBuffer cmd = _get_current_frame().command_buffer;

	const bool msaa_used = _msaa_samples != 1;

	// Get final image to copy
	Image final_color_attachment = GL_NULL_HANDLE;
	if (_swapchain_target_image_id.empty()) {
		for (const auto& [id, render_image] : _renderpass_images) {
			if (render_image.is_depth_attachment) {
				continue;
			}

			final_color_attachment = render_image.image;
		}
	} else {
		const auto final_color_attachment_it = _renderpass_images.find(_swapchain_target_image_id);
		if (final_color_attachment_it != _renderpass_images.end()) {
			final_color_attachment = final_color_attachment_it->second.image;
		}
	}

	// TODO!
	if (!final_color_attachment) {
		GL_LOG_ERROR("[Renderer::end_render] Unable to find color attachment to render");
		return;
	}

	const Image render_target =
			_should_present_to_swapchain ? _current_swapchain_image : _final_image;

	// Copy color image to final image if there isn't got a resolver
	if (!msaa_used) {
		s_device->command_transition_image(cmd, final_color_attachment,
				ImageLayout::COLOR_ATTACHMENT_OPTIMAL, ImageLayout::TRANSFER_SRC_OPTIMAL);
		s_device->command_transition_image(cmd, render_target,
				ImageLayout::COLOR_ATTACHMENT_OPTIMAL, ImageLayout::TRANSFER_DST_OPTIMAL);

		Vec2u src_size = s_device->image_get_size(final_color_attachment).value();
		Vec2u dst_size = s_device->image_get_size(render_target).value();
		s_device->command_copy_image_to_image(
				cmd, final_color_attachment, render_target, src_size, dst_size);
	}

	// Transition into shader read only to use as a viewport image
	// TODO: Re-implement imgui integration
	// Do this before imgui, if final_image being used as an imgui image
	// image layout must be set to SHADER_READ_ONLY_OPTIMAL
	if (!_should_present_to_swapchain) {
		s_device->command_transition_image(cmd, _final_image,
				!msaa_used ? ImageLayout::TRANSFER_DST_OPTIMAL
						   : ImageLayout::COLOR_ATTACHMENT_OPTIMAL,
				ImageLayout::SHADER_READ_ONLY_OPTIMAL);
	}

	if (_imgui_being_used) {
		if (_should_present_to_swapchain && !msaa_used) {
			s_device->command_transition_image(cmd, _current_swapchain_image,
					ImageLayout::TRANSFER_DST_OPTIMAL, ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
		}

		// TODO: Re-implement imgui integration
		// _imgui_pass(cmd, _current_swapchain_image);
	}

	s_device->command_transition_image(cmd, _current_swapchain_image,
			(_should_present_to_swapchain && !msaa_used && !_imgui_being_used)
					? ImageLayout::TRANSFER_DST_OPTIMAL
					: ImageLayout::COLOR_ATTACHMENT_OPTIMAL,
			ImageLayout::PRESENT_SRC);

	s_device->command_end(cmd);

	s_device->queue_submit(_graphics_queue, cmd, _get_current_frame().render_fence,
			_get_current_frame().image_available_semaphore,
			_get_current_frame().render_finished_semaphore);

	auto present_result = s_device->queue_present(
			_present_queue, _swapchain, _get_current_frame().render_finished_semaphore);
	if (!present_result) {
		_request_resize();
	}

	// reset the state
	_imgui_being_used = false;
	_current_swapchain_image = nullptr;
	_frame_number++;
}

void Renderer::add_pass(std::shared_ptr<GraphicsPass> pass, int priority) {
	pass->setup(*this);

	_graphics_passes.push_back(std::make_pair(pass, priority));
}

void Renderer::execute(CommandBuffer cmd) {
	// Sort based on priority
	std::sort(_graphics_passes.begin(), _graphics_passes.end(),
			[](const auto& lhs, const auto& rhs) -> bool { return lhs.second < rhs.second; });

	for (const auto& [pass, _prior] : _graphics_passes) {
		if (!pass->is_active()) {
			continue;
		}

		pass->execute(cmd, *this);
	}
}

void Renderer::begin_rendering(CommandBuffer cmd, Image color_attachment, Image depth_attachment,
		std::optional<Color> clear_color) {
	RenderingAttachment attachment = {};
	attachment.image = color_attachment;
	attachment.layout = ImageLayout::COLOR_ATTACHMENT_OPTIMAL;

	if (clear_color) {
		attachment.load_op = AttachmentLoadOp::CLEAR;
		attachment.clear_color = *clear_color;
	} else {
		attachment.load_op = AttachmentLoadOp::LOAD;
	}

	if (_msaa_samples != 1) {
		attachment.resolve_mode = RESOLVE_MODE_AVERAGE_BIT;
		attachment.resolve_image =
				_should_present_to_swapchain ? _current_swapchain_image : _final_image;
		attachment.resolve_layout = ImageLayout::COLOR_ATTACHMENT_OPTIMAL;
	}

	Vec2u attachment_size = s_device->image_get_size(color_attachment).value();
	s_device->command_begin_rendering(cmd, attachment_size, attachment, depth_attachment);
}

void Renderer::end_rendering(CommandBuffer cmd) { s_device->command_end_rendering(cmd); }

Result<Image, Renderer::ImageCreateError> Renderer::create_render_image(
		const std::string& name, DataFormat format, ImageUsageFlags usage) {
	if (_renderpass_images.find(name) != _renderpass_images.end()) {
		return make_err<Image>(ImageCreateError::ID_EXISTS);
	}

	if (!(usage & IMAGE_USAGE_TRANSFER_SRC_BIT)) {
		usage |= IMAGE_USAGE_TRANSFER_SRC_BIT;
	}
	if (!(usage & IMAGE_USAGE_TRANSFER_DST_BIT)) {
		usage |= IMAGE_USAGE_TRANSFER_DST_BIT;
	}

	const bool is_depth_format = format == DataFormat::D16_UNORM ||
			format == DataFormat::D16_UNORM_S8_UINT || format == DataFormat::D24_UNORM_S8_UINT ||
			format == DataFormat::D32_SFLOAT;

	RenderImage render_image;
	render_image.image = s_device->image_create(ImageCreateInfo{ .format = format,
														.size = get_resolution_extent(),
														.data = nullptr,
														.usage = usage,
														.samples = _msaa_samples })
								 .value();
	render_image.format = format;
	render_image.usage = usage;
	render_image.is_depth_attachment = is_depth_format;

	// bookkeep
	_renderpass_images[name] = render_image;

	return render_image.image;
}

std::optional<Image> Renderer::get_render_image(const std::string& name) {
	const auto it = _renderpass_images.find(name);
	if (it == _renderpass_images.end()) {
		return {};
	}

	return it->second.image;
}

void Renderer::set_swapchain_target(const std::string& name) { _swapchain_target_image_id = name; }

void Renderer::wait_for_device() { s_device->device_wait(); }

void Renderer::imgui_begin() {
	// GL_PROFILE_SCOPE;

	_imgui_being_used = true;

	// TODO: Re-implement imgui integration
	// s_device->imgui_new_frame_for_platform();

	ImGui::NewFrame();
}

void Renderer::imgui_end() {
	// GL_PROFILE_SCOPE;

	ImGui::Render();
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}

void Renderer::set_render_present_mode(bool present_to_swapchain) {
	_should_present_to_swapchain = present_to_swapchain;
}

float Renderer::get_resolution_scale() const { return _settings.resolution_scale; }

void Renderer::set_resolution_scale(float scale) {
	if (fabs(scale - _settings.resolution_scale) > 0.001f) {
		_settings.resolution_scale = scale;

		_request_resize();
	}
}

bool Renderer::get_vsync_enabled() const { return _settings.vsync; }

void Renderer::set_vsync(bool vsync) {
	// avoid unnecessary resize
	if (_settings.vsync != vsync) {
		_settings.vsync = vsync;

		_request_resize();
	}
}

uint32_t Renderer::get_msaa_samples() const { return _msaa_samples; }

void Renderer::set_msaa_samples(uint32_t samples) {
	const uint32_t max_sample_count = s_device->get_max_msaa_samples();

	if ((samples != 1 && samples % 2 != 0) || samples > max_sample_count) {
		GL_LOG_ERROR("[Renderer::set_msaa_samples] Invalid MSAA sample count: {}. Must be 1 or "
					 "power-of-two, and ≤ {}",
				samples, max_sample_count);
		_msaa_samples = 1;
		return;
	}

	if (_msaa_samples != samples) {
		_msaa_samples = samples;

		_request_resize();
	}
}

Swapchain Renderer::get_swapchain() { return _swapchain; }

Vec2u Renderer::get_resolution_extent() const {
	const Vec2u swapchain_size = s_device->swapchain_get_extent(_swapchain).value();
	return {
		std::max(1u, uint32_t(swapchain_size.x * _settings.resolution_scale)),
		std::max(1u, uint32_t(swapchain_size.y * _settings.resolution_scale)),
	};
}

void* Renderer::get_final_image_descriptor() const { return _final_image_descriptor; }

Vec2u Renderer::get_final_image_size() const {
	return s_device->image_get_size(_final_image).value();
}

RenderStats& Renderer::get_stats() { return _stats; }

Device* Renderer::get_device() { return s_device.get(); }

void Renderer::_imgui_pass(CommandBuffer cmd, Image target_image) {
	// TODO: Re-implement imgui integration
	/*
		// GL_PROFILE_SCOPE;

	RenderingAttachment color_attachment = {};
	color_attachment.image = p_target_image;
	color_attachment.layout = ImageLayout::COLOR_ATTACHMENT_OPTIMAL;

	Vec2u target_size = s_device->image_get_size(p_target_image).value();
	s_device->command_begin_rendering(
			p_cmd, target_size, color_attachment);

	s_device->imgui_render_for_platform(p_cmd);

	s_device->command_end_rendering(p_cmd);
	*/
}

void Renderer::_imgui_init() {
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	(void)io;

	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;
	io.IniFilename = ".glitch/imgui.ini";

	io.Fonts->Clear();

	// TODO: Re-implement imgui integration
	// s_device->imgui_init_for_platform(
	//		window->get_native_window(), s_device->swapchain_get_format(_swapchain));

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
	// GL_PROFILE_SCOPE_N("Renderer::Swapchain Resize");

	const Vec2u window_px = _window->get_size();
	s_device->swapchain_resize(_graphics_queue, _swapchain, window_px, _settings.vsync);

	const Vec2u new_size = get_resolution_extent();

	// Resize depth, color and final image
	if (_final_image) {
		s_device->image_free(_final_image);
	}
	_final_image = s_device
						   ->image_create(ImageCreateInfo{
								   .format = s_device->swapchain_get_format(_swapchain).value(),
								   .size = new_size,
								   .data = nullptr,
								   .usage = IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
										   IMAGE_USAGE_SAMPLED_BIT | IMAGE_USAGE_TRANSFER_DST_BIT })
						   .value();

	if (_final_image_descriptor) {
		// TODO: Re-implement imgui integration
		// s_device->imgui_image_free(_final_image_descriptor);
	}
	// TODO: Re-implement imgui integration
	// _final_image_descriptor = s_device->imgui_image_upload(_final_image, _default_sampler);

	// Recreate renderpass attachments
	for (auto& [id, render_image] : _renderpass_images) {
		if (render_image.image) {
			s_device->image_free(render_image.image);
		}

		render_image.image = s_device->image_create(ImageCreateInfo{ .format = render_image.format,
															.size = new_size,
															.data = nullptr,
															.usage = render_image.usage,
															.samples = _msaa_samples })
									 .value();
	}
}

void Renderer::_reset_stats() { memset(&_stats, 0, sizeof(RenderStats)); }

} //namespace gl
