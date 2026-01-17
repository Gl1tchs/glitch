/**
 * @file application.h
 */

#pragma once

#include "glitch/core/layer_stack.h"
#include "glitch/core/window.h"
#include "glitch/renderer/renderer.h"

namespace gl {

struct ApplicationPerfStats {
	float delta_time;

	struct {
		uint32_t draw_calls;
		uint32_t index_count;
	} renderer_stats;
};

struct ApplicationCreateInfo {
	const char* name;
	VectorView<const char*> args;
};

typedef std::function<void(void)> MainThreadFunc;

class GL_API Application {
public:
	Application(const ApplicationCreateInfo& info);
	virtual ~Application();

	void run();

	void quit();

	template <typename T, typename... Args>
	void push_layer(Args&&... args)
		requires std::is_base_of_v<Layer, T>
	{
		_layer_stack.push_layer<T>(std::forward<Args>(args)...);
	}

	/**
	 * Enqueue a function to be runned for the
	 * next frame.
	 */
	static void enqueue_main_thread(MainThreadFunc function);

	std::shared_ptr<Window> get_window();

	std::shared_ptr<Renderer> get_renderer();

	// TODO: maybe this shouldn't be mutable
	ApplicationPerfStats& get_perf_stats();

	static Application* get();

private:
	void _event_loop(float dt);

	void _process_main_thread_queue();

private:
	bool _running = true;

	LayerStack _layer_stack;

	std::shared_ptr<Window> _window = nullptr;
	std::shared_ptr<Renderer> _renderer = nullptr;

	std::vector<MainThreadFunc> _main_thread_queue;
	std::mutex _main_thread_queue_mutex;

	ApplicationPerfStats _perf_stats = {};
};

} //namespace gl
