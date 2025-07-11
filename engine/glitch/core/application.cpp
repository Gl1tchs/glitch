#include "glitch/core/application.h"

#include "glitch/core/event/event_system.h"
#include "glitch/core/timer.h"
#include "glitch/renderer/material.h"
#include "glitch/renderer/material_definitions.h"

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

	MaterialSystem::init();

	// Register material definitions
	MaterialSystem::register_definition(
			"unlit_standart", get_unlit_material_definition());
	MaterialSystem::register_definition(
			"urp_standart", get_urp_material_definition());
}

Application::~Application() {
	renderer->wait_for_device();

	// Destroy material system
	MaterialSystem::destroy();
}

void Application::run() {
	_on_start();

	Timer timer;
	while (running) {
		const float dt = timer.get_delta_time();

		perf_stats = {};
		perf_stats.delta_time = dt;

		_event_loop(dt);
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
	GL_PROFILE_SCOPE;

	window->poll_events();

	_process_main_thread_queue();

	{
		GL_PROFILE_SCOPE_N("Application::_on_update");

		_on_update(p_dt);
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

Application* Application::get_instance() { return s_instance; }
