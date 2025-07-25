/**
 * @file application.h
 */

#pragma once

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
	int argc;
	const char** argv;
};

typedef std::function<void(void)> MainThreadFunc;

class GL_API Application {
public:
	Application(const ApplicationCreateInfo& p_info);
	virtual ~Application();

	void run();

	void quit();

	/**
	 * Enqueue a function to be runned for the
	 * next frame.
	 */
	static void enqueue_main_thread(MainThreadFunc p_function);

	Ref<Window> get_window() { return window; }

	Ref<Renderer> get_renderer() { return renderer; }

	// TODO: maybe this shouldn't be mutable
	ApplicationPerfStats& get_perf_stats() { return perf_stats; }

	static Application* get_instance();

private:
	void _event_loop(float p_dt);

	void _process_main_thread_queue();

protected:
	inline virtual void _on_start() {}

	inline virtual void _on_update(float p_dt) {}

	inline virtual void _on_destroy() {}

private:
	static Application* s_instance;

	bool running = true;

	Ref<Window> window;
	Ref<Renderer> renderer;

	std::vector<MainThreadFunc> main_thread_queue;
	std::mutex main_thread_queue_mutex;

	ApplicationPerfStats perf_stats = {};
};

} //namespace gl