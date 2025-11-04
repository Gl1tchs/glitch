#include "glitch/core/application.h"

#include "glitch/core/event/event_system.h"
#include "glitch/core/timer.h"
#include "glitch/renderer/material.h"

namespace gl {

static Application* s_instance = nullptr;

Application::Application(const ApplicationCreateInfo& p_info) {
	GL_ASSERT(!s_instance, "Only one instance can exists at a time!");
	s_instance = this;

	WindowCreateInfo window_info = {};
	window_info.title = p_info.name;
	window = std::make_shared<Window>(window_info);

	event::subscribe<WindowCloseEvent>([this](const auto& _event) { running = false; });

	// initialize render backend
	renderer = std::make_shared<Renderer>(window);
}

Application::~Application() {
	renderer->wait_for_device();

	// Destroy material system
	MaterialSystem::destroy();
}

void Application::run() {
	Timer timer;
	while (running) {
		const float dt = timer.get_delta_time();

		perf_stats = {};
		perf_stats.delta_time = dt;

		_event_loop(dt);
	}

	// Clear and destroy the layers
	layer_stack.clear();
}

void Application::quit() { running = false; }

void Application::enqueue_main_thread(MainThreadFunc p_function) {
	Application* app = get_instance();
	if (!app) {
		return;
	}

	std::scoped_lock<std::mutex> lock(app->main_thread_queue_mutex);
	app->main_thread_queue.push_back(p_function);
}

std::shared_ptr<Window> Application::get_window() { return window; }

std::shared_ptr<Renderer> Application::get_renderer() { return renderer; }

ApplicationPerfStats& Application::get_perf_stats() { return perf_stats; }

std::shared_ptr<RenderBackend> Application::get_render_backend() { return Renderer::get_backend(); }

Application* Application::get_instance() { return s_instance; }

void Application::_event_loop(float p_dt) {
	GL_PROFILE_SCOPE;

	window->poll_events();

	_process_main_thread_queue();

	{
		GL_PROFILE_SCOPE_N("Application::_on_update");

		for (const auto& layer : layer_stack) {
			layer->update(p_dt);
		}
	}
}

void Application::_process_main_thread_queue() {
	GL_PROFILE_SCOPE;

	std::scoped_lock<std::mutex> lock(main_thread_queue_mutex);
	for (auto& func : main_thread_queue) {
		func();
	}

	main_thread_queue.clear();
}

} //namespace gl
