#pragma once

#include "core/window.h"
#include "renderer/renderer.h"

struct ApplicationCreateInfo {
	const char* name;
	int argc;
	const char** argv;
};

typedef std::function<void(void)> MainThreadFunc;

class Application {
public:
	Application(const ApplicationCreateInfo& p_info);
	virtual ~Application() = default;

	void run();

	void quit();

	Ref<Window> get_window();

	Ref<Renderer> get_renderer();

	static void enqueue_main_thread(MainThreadFunc p_function);

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
};
