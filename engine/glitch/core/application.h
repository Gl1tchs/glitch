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
	Application(const ApplicationCreateInfo& p_info);
	virtual ~Application();

	void run();

	void quit();

	template <typename T, typename... Args>
	void push_layer(Args&&... p_args)
		requires std::is_base_of_v<Layer, T>
	{
		layer_stack.push_layer<T>(std::forward<Args>(p_args)...);
	}

	/**
	 * Enqueue a function to be runned for the
	 * next frame.
	 */
	static void enqueue_main_thread(MainThreadFunc p_function);

	std::shared_ptr<Window> get_window();

	std::shared_ptr<Renderer> get_renderer();

	// TODO: maybe this shouldn't be mutable
	ApplicationPerfStats& get_perf_stats();

	static Application* get();

private:
	void _event_loop(float p_dt);

	void _process_main_thread_queue();

private:
	bool running = true;

	LayerStack layer_stack;

	std::shared_ptr<Window> window = nullptr;
	std::shared_ptr<Renderer> renderer = nullptr;

	std::vector<MainThreadFunc> main_thread_queue;
	std::mutex main_thread_queue_mutex;

	ApplicationPerfStats perf_stats = {};
};

} //namespace gl
