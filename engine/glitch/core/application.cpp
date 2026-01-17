#include "glitch/core/application.h"

#include "glitch/asset/asset_system.h"
#include "glitch/core/debug/profiling.h"
#include "glitch/core/event/event_system.h"
#include "glitch/core/timer.h"
#include "glitch/scripting/script_engine.h"

#include <mutex>

namespace gl {

static Application* s_instance = nullptr;

Application::Application(const ApplicationCreateInfo& info) {
	GL_ASSERT(!s_instance, "Only one instance can exists at a time!");
	s_instance = this;

	// Initialize core systems (window / events)

	WindowCreateInfo window_info = {};
	window_info.title = info.name;
	_window = std::make_shared<Window>(window_info);

	event::subscribe<WindowCloseEvent>([this](const auto& _event) { _running = false; });

	// Initialize the renderer

	_renderer = std::make_shared<Renderer>(_window);

	// System initialization

	ScriptEngine::init();
}

Application::~Application() {
	_renderer->wait_for_device();

	// Destroy systems
	AssetSystem::clear();
	ScriptEngine::shutdown();
}

void Application::run() {
	Timer timer;
	while (_running) {
		const float dt = timer.get_delta_time();

		_perf_stats = {};
		_perf_stats.delta_time = dt;

		_event_loop(dt);
	}

	// Clear and destroy the layers
	_layer_stack.clear();
}

void Application::quit() { _running = false; }

void Application::enqueue_main_thread(MainThreadFunc function) {
	Application* app = Application::get();
	if (!app) {
		return;
	}

	std::scoped_lock<std::mutex> lock(app->_main_thread_queue_mutex);
	app->_main_thread_queue.push_back(function);
}

std::shared_ptr<Window> Application::get_window() { return _window; }

std::shared_ptr<Renderer> Application::get_renderer() { return _renderer; }

ApplicationPerfStats& Application::get_perf_stats() { return _perf_stats; }

Application* Application::get() { return s_instance; }

void Application::_event_loop(float dt) {
	GL_PROFILE_SCOPE;

	_window->poll_events();

	_process_main_thread_queue();

	{
		GL_PROFILE_SCOPE_N("Application::_on_update");

		for (const auto& layer : _layer_stack) {
			layer->update(dt);
		}
	}
}

void Application::_process_main_thread_queue() {
	GL_PROFILE_SCOPE;

	std::scoped_lock<std::mutex> lock(_main_thread_queue_mutex);
	for (auto& func : _main_thread_queue) {
		func();
	}

	_main_thread_queue.clear();
}

} //namespace gl
