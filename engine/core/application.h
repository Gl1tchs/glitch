#pragma once

#include "core/window.h"
#include "renderer/material.h"
#include "renderer/mesh.h"
#include "renderer/renderer.h"

struct ApplicationCreateInfo {
	const char* name;
	int argc;
	const char** argv;
};

typedef std::function<void(void)> MainThreadFunc;

class Application {
public:
	Application(const ApplicationCreateInfo& info);
	virtual ~Application();

	void run();

	void quit();

	Ref<Window> get_window();

	static void enqueue_main_thread(MainThreadFunc func);

	static Application* get_instance();

private:
	void _event_loop(float dt);

	void _process_main_thread_queue();

protected:
	inline virtual void _on_start() {}

	inline virtual void _on_update(float dt) {}

	inline virtual void _on_imgui_update(float dt) {}

	inline virtual void _on_destroy() {}

private:
	static Application* s_instance;

	bool running = true;

	Ref<Window> window;
	Ref<Renderer> renderer;

	// TEMP
	Ref<Mesh> mesh;
	Ref<MetallicRoughnessMaterial> material;

	Ref<MaterialInstance> material_instance;
	Ref<MaterialInstance> material_instance2;

	Ref<MaterialInstance> current_material;
	bool space_pressed = false;

	Ref<Image> color_image;
	Ref<Image> color_image2;
	Ref<Image> white_image;

	std::vector<MainThreadFunc> main_thread_queue;
	std::mutex main_thread_queue_mutex;
};
