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
	VectorView<const char*> args;
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

	Ref<Window> get_window();

	Ref<Renderer> get_renderer();

	// TODO: maybe this shouldn't be mutable
	ApplicationPerfStats& get_perf_stats();

	static Ref<RenderBackend> get_render_backend();

	static Application* get_instance();

private:
	void _event_loop(float p_dt);

	void _process_main_thread_queue();

protected:
	inline virtual void _on_start() {}

	inline virtual void _on_update(float p_dt) {}

	inline virtual void _on_destroy() {}

private:
	bool running = true;

	Ref<Window> window = nullptr;
	Ref<Renderer> renderer = nullptr;

	std::vector<MainThreadFunc> main_thread_queue;
	std::mutex main_thread_queue_mutex;

	ApplicationPerfStats perf_stats = {};
};

} //namespace gl
