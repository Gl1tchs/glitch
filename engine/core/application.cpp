#include "core/application.h"

#include "core/event_system.h"
#include "core/timer.h"

Application* Application::s_instance = nullptr;

Application::Application(const ApplicationCreateInfo& info) : render_ctx() {
	GL_ASSERT(!s_instance, "Only on instance can exists at a time!");
	s_instance = this;

	WindowCreateInfo window_info{};
	window_info.title = info.name;
	window = create_ref<Window>(window_info);

	event::subscribe<WindowCloseEvent>(
			[this](const auto& _event) { running = false; });

	// initialize render backend
	render_ctx.backend = find_proper_backend();
	render_ctx.init(window);
}

Application::~Application() { render_ctx.destroy(); }

void Application::run() {
	_on_start();

	Timer timer;
	while (running) {
		_event_loop(timer.get_delta_time());
	}

	_on_destroy();
}

void Application::enqueue_main_thread(MainThreadFunc func) {
	Application* app = get_instance();
	if (!app) {
		return;
	}

	std::scoped_lock<std::mutex> lock(app->main_thread_queue_mutex);
	app->main_thread_queue.push_back(func);
}

Application* Application::get_instance() { return s_instance; }

void Application::_event_loop(float dt) {
	window->poll_events();

	_process_main_thread_queue();

	_on_update(dt);
}

void Application::_process_main_thread_queue() {
	std::scoped_lock<std::mutex> lock(main_thread_queue_mutex);
	for (auto& func : main_thread_queue) {
		func();
	}

	main_thread_queue.clear();
}

void Application::quit() { running = false; }

Ref<Window> Application::get_window() { return window; }
