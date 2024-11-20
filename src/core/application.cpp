#include "glitch/core/application.h"

#include "glitch/core/timer.h"

#include "core/event/event_system.h"

Application* Application::s_instance = nullptr;

Application::Application(const ApplicationCreateInfo& p_info) {
	GL_ASSERT(!s_instance, "Only one instance can exists at a time!");
	s_instance = this;

	WindowCreateInfo window_info{};
	window_info.title = p_info.name;
	window = create_ref<Window>(window_info);

	event::subscribe<WindowCloseEvent>(
			[this](const auto& _event) { running = false; });

	// initialize render backend
	renderer = create_ref<Renderer>(window);
}

void Application::run() {
	_on_start();

	Timer timer;
	while (running) {
		_event_loop(timer.get_delta_time());
	}

	_on_destroy();
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

void Application::_event_loop(float p_dt) {
	window->poll_events();

	_process_main_thread_queue();

	_on_update(p_dt);
}

void Application::_process_main_thread_queue() {
	std::scoped_lock<std::mutex> lock(main_thread_queue_mutex);
	for (auto& func : main_thread_queue) {
		func();
	}

	main_thread_queue.clear();
}

Application* Application::get_instance() { return s_instance; }
