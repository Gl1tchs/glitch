#include "core/application.h"

#include "core/event_system.h"
#include "core/input.h"
#include "core/timer.h"
#include "renderer/image.h"
#include "renderer/material.h"

// temp
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <glm/packing.hpp>
// end temp

Application* Application::s_instance = nullptr;

Application::Application(const ApplicationCreateInfo& info) {
	GL_ASSERT(!s_instance, "Only on instance can exists at a time!");
	s_instance = this;

	WindowCreateInfo window_info{};
	window_info.title = info.name;
	window = create_ref<Window>(window_info);

	event::subscribe<WindowCloseEvent>(
			[this](const auto& _event) { running = false; });

	// initialize render backend
	auto backend = find_proper_backend();
	renderer = Renderer::create(backend, window);

	std::vector<Vertex> vertices = {
		{ { 0.5f, 0.5f, 0.0f }, 1.0f, { 0.0f, 0.0f, 0.0f }, 1.0f,
				{ 0.9f, 0.6f, 0.34f, 1.0f } },
		{ { 0.5f, -0.5f, 0.0f }, 1.0f, { 0.0f, 0.0f, 0.0f }, 0.0f,
				{ 0.2f, 0.4f, 0.8f, 1.0f } },
		{ { -0.5f, -0.5f, 0.0f }, 0.0f, { 0.0f, 0.0f, 0.0f }, 0.0f,
				{ 0.4f, 0.8f, 0.6f, 1.0f } },
		{ { -0.5f, 0.5f, 0.0f }, 0.0f, { 0.0f, 0.0f, 0.0f }, 1.0f,
				{ 0.8f, 0.2f, 0.4f, 1.0f } },
	};

	std::vector<uint32_t> indices = { 0, 1, 2, 2, 3, 0 };

	mesh = Mesh::create(vertices, indices);

	material = MetallicRoughnessMaterial::create();

	MetallicRoughnessMaterial::MaterialConstants constants = {
		.color_factors = { 1.0f, 1.0f, 1.0, 1.0f },
		.metal_rough_factors = { 1.0f, 0.5f, 1.0f, 1.0f },
	};

	uint32_t white = glm::packUnorm4x8(glm::vec4(1, 1, 1, 1));
	white_image = Image::create(
			(void*)&white, Vec2u{ 1, 1 }, ImageFormat::R8G8B8A8_UNORM);

	Vec2u size = {};
	int channel_count;
	uint8_t* image_data = stbi_load("assets/t_texture.png", (int*)&size.x,
			(int*)&size.y, &channel_count, STBI_rgb_alpha);

	color_image = Image::create(image_data, size, ImageFormat::R8G8B8A8_UNORM);

	stbi_image_free(image_data);

	image_data = stbi_load("assets/texture.jpg", (int*)&size.x, (int*)&size.y,
			&channel_count, STBI_rgb_alpha);

	color_image2 = Image::create(image_data, size, ImageFormat::R8G8B8A8_UNORM);

	stbi_image_free(image_data);

	MetallicRoughnessMaterial::MaterialResources resources = {
		.constants = constants,
		.constants_offset = 0,
		.color_image = color_image,
		.color_filtering = ImageFilteringMode::LINEAR,
		.roughness_image = white_image,
		.roughness_filtering = ImageFilteringMode::LINEAR,
	};
	material_instance = material->create_instance(resources);

	resources.color_image = color_image2;

	material_instance2 = material->create_instance(resources);

	current_material = material_instance;
}

Application::~Application() {
	// wait for operations to finish
	// before cleaning other resources
	renderer->wait_for_device();

	Mesh::destroy(mesh);

	Image::destroy(color_image);
	Image::destroy(color_image2);
	Image::destroy(white_image);

	MetallicRoughnessMaterial::destroy(material);
}

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

	if (!space_pressed && Input::is_key_pressed(KeyCode::SPACE)) {
		if (current_material == material_instance) {
			current_material = material_instance2;
		} else {
			current_material = material_instance;
		}
		space_pressed = true;
	}
	if (space_pressed && Input::is_key_released(KeyCode::SPACE)) {
		space_pressed = false;
	}

	renderer->submit_mesh(mesh, current_material);

	renderer->draw();
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
