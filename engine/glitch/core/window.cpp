#include "glitch/core/window.h"

#include "glitch/core/event/event_system.h"
#include "glitch/core/event/input.h"
#include "glitch/core/event/key_code.h"
#include "glitch/core/event/mouse_button.h"

#include <GLFW/glfw3.h>

namespace gl {

static void _glfw_error_callback(int p_error, const char* p_description) {
	GL_LOG_ERROR("GLFW error {}: {}.", p_error, p_description);
}

Window::Window(WindowCreateInfo p_info) {
	GL_ASSERT(glfwInit());

#if GL_DEBUG_BUILD
	glfwSetErrorCallback(_glfw_error_callback);
#endif

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

	window = glfwCreateWindow(
			p_info.w, p_info.h, p_info.title, nullptr, nullptr);
	GL_ASSERT(window);

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

glm::uvec2 Window::get_size() const {
	glm::uvec2 s{};
	glfwGetWindowSize(window, (int*)&s.x, (int*)&s.y);
	return s;
}

float Window::get_aspect_ratio() const {
	const glm::uvec2 s = get_size();
	return static_cast<float>(s.x) / static_cast<float>(s.y);
}

void Window::set_title(std::string_view p_title) {
	glfwSetWindowTitle(window, p_title.data());
}

WindowCursorMode Window::get_cursor_mode() const { return cursor_mode; }

void Window::set_cursor_mode(WindowCursorMode p_mode) {
	cursor_mode = p_mode;

	switch (cursor_mode) {
		case WINDOW_CURSOR_MODE_NORMAL:
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			break;
		case WINDOW_CURSOR_MODE_HIDDEN:
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
			break;
		case WINDOW_CURSOR_MODE_DISABLED:
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			break;
		case WINDOW_CURSOR_MODE_CAPTURED:
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
								static_cast<MouseButton>(button);
						event::notify(mouse_event);
						break;
					}
					case GLFW_RELEASE: {
						MouseReleaseEvent mouse_event{};
						mouse_event.button_code =
								static_cast<MouseButton>(button);
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

} //namespace gl