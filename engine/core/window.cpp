#include "core/window.h"

#include "core/event_system.h"
#include "core/input.h"
#include "core/key_code.h"
#include "core/mouse_code.h"

#include <GLFW/glfw3.h>

static void glfw_error_callback(int error, const char* description) {
	GL_LOG_ERROR("GLFW error {}: {}.", error, description);
}

Window::Window(WindowCreateInfo info) {
	GL_ASSERT(glfwInit());

#if GL_DEBUG
	glfwSetErrorCallback(glfw_error_callback);
#endif

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	// not resizable at the moment
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

	window = glfwCreateWindow(info.w, info.h, info.title, nullptr, nullptr);
	GL_ASSERT(window);

	glfwMakeContextCurrent(window);

	// initialize event system
	_assign_event_delegates();

	// initialize input
	Input::init();
}

Window::~Window() {
	glfwDestroyWindow(window);
	glfwTerminate();
}

void Window::poll_events() const { glfwPollEvents(); }

bool Window::is_open() const { return !glfwWindowShouldClose(window); }

Vector2i Window::get_size() const {
	Vector2i s{};
	glfwGetWindowSize(window, &s.x, &s.y);
	return s;
}

float Window::get_aspect_ratio() const {
	const Vector2i s = get_size();
	return static_cast<float>(s.x) / static_cast<float>(s.y);
}

void Window::set_title(std::string_view title) {
	glfwSetWindowTitle(window, title.data());
}

WindowCursorMode Window::get_cursor_mode() const { return cursor_mode; }

void Window::set_cursor_mode(WindowCursorMode mode) {
	cursor_mode = mode;

	switch (cursor_mode) {
		case WindowCursorMode::NORMAL:
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			break;
		case WindowCursorMode::HIDDEN:
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
			break;
		case WindowCursorMode::DISABLED:
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			break;
		case WindowCursorMode::CAPTURED:
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_CAPTURED);
			break;
		default:
			break;
	}
}

GLFWwindow* Window::get_native_window() { return window; }

void Window::_assign_event_delegates() {
	glfwSetWindowSizeCallback(
			window, [](GLFWwindow* window, int width, int height) {
				WindowResizeEvent resize_event{};
				resize_event.size = { width, height };
				event::notify(resize_event);
			});

	glfwSetWindowCloseCallback(window, [](GLFWwindow* window) {
		WindowCloseEvent close_event{};
		event::notify<WindowCloseEvent>(close_event);
	});

	glfwSetKeyCallback(window,
			[](GLFWwindow* window, int key, int scancode, int action,
					int mods) {
				switch (action) {
					case GLFW_PRESS: {
						KeyPressEvent key_event{};
						key_event.key_code = static_cast<KeyCode>(key);
						event::notify(key_event);
						break;
					}
					case GLFW_RELEASE: {
						KeyReleaseEvent key_event{};
						key_event.key_code = static_cast<KeyCode>(key);
						event::notify(key_event);
						break;
					}
					default: {
						break;
					}
				}
			});

	glfwSetCharCallback(window, [](GLFWwindow* window, unsigned int keycode) {
		KeyTypeEvent type_event{};
		type_event.key_code = static_cast<KeyCode>(keycode);
		event::notify(type_event);
	});

	glfwSetMouseButtonCallback(
			window, [](GLFWwindow* window, int button, int action, int mods) {
				switch (action) {
					case GLFW_PRESS: {
						MousePressEvent mouse_event{};
						mouse_event.button_code =
								static_cast<MouseCode>(button);
						event::notify(mouse_event);
						break;
					}
					case GLFW_RELEASE: {
						MouseReleaseEvent mouse_event{};
						mouse_event.button_code =
								static_cast<MouseCode>(button);
						event::notify(mouse_event);
						break;
					}
					default: {
						break;
					}
				}
			});

	glfwSetCursorPosCallback(window,
			[](GLFWwindow* window, const double x_pos, const double y_pos) {
				MouseMoveEvent move_event{};
				move_event.position = { static_cast<float>(x_pos),
					static_cast<float>(y_pos) };
				event::notify(move_event);
			});

	glfwSetScrollCallback(window,
			[](GLFWwindow* window, const double x_offset,
					const double y_offset) {
				MouseScrollEvent scroll_event{};
				scroll_event.offset = { static_cast<float>(x_offset),
					static_cast<float>(y_offset) };
				event::notify(scroll_event);
			});
}
