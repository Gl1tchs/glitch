#include "glitch/core/window.h"

#include "glitch/core/event/event_system.h"
#include "glitch/core/event/input.h"
#include "glitch/core/event/key_code.h"
#include "glitch/core/event/mouse_button.h"

#include <glgpu/glgpu.h>

#include <GLFW/glfw3.h>

namespace gl {

static void _glfw_error_callback(int error, const char* description) {
	GL_LOG_ERROR("[GLFW] Code {}: {}.", error, description);
}

Window::Window(WindowCreateInfo info) {
	GL_ASSERT(glfwInit());

#if GL_DEBUG_BUILD
	glfwSetErrorCallback(_glfw_error_callback);
#endif

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

	_window = glfwCreateWindow(info.w, info.h, info.title, nullptr, nullptr);
	GL_ASSERT(_window);

	// initialize event system
	_assign_event_delegates();

	// initialize input
	Input::init();
}

Window::~Window() {
	glfwDestroyWindow(_window);
	glfwTerminate();
}

void Window::poll_events() const { glfwPollEvents(); }

bool Window::is_open() const { return !glfwWindowShouldClose(_window); }

Vec2u Window::get_size() const {
	int w, h;
	glfwGetWindowSize(_window, &w, &h);
	return Vec2u(static_cast<uint32_t>(w), static_cast<uint32_t>(h));
}

float Window::get_aspect_ratio() const {
	const Vec2u s = get_size();
	return static_cast<float>(s.x) / static_cast<float>(s.y);
}

void Window::set_title(std::string_view title) { glfwSetWindowTitle(_window, title.data()); }

WindowCursorMode Window::get_cursor_mode() const { return _cursor_mode; }

void Window::set_cursor_mode(WindowCursorMode mode) {
	_cursor_mode = mode;

	switch (_cursor_mode) {
		case WINDOW_CURSOR_MODE_NORMAL:
			glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			break;
		case WINDOW_CURSOR_MODE_HIDDEN:
			glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
			break;
		case WINDOW_CURSOR_MODE_DISABLED:
			glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			break;
		case WINDOW_CURSOR_MODE_CAPTURED:
			glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_CAPTURED);
			break;
		default:
			break;
	}
}

GLFWwindow* Window::get_native_window() { return _window; }

void Window::_assign_event_delegates() {
	glfwSetWindowSizeCallback(_window, [](GLFWwindow* window, int width, int height) {
		WindowResizeEvent resize_event{};
		resize_event.size = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
		event::notify(resize_event);
	});

	glfwSetWindowCloseCallback(_window, [](GLFWwindow* window) {
		WindowCloseEvent close_event{};
		event::notify<WindowCloseEvent>(close_event);
	});

	glfwSetKeyCallback(
			_window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
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

	glfwSetCharCallback(_window, [](GLFWwindow* window, unsigned int keycode) {
		KeyTypeEvent type_event{};
		type_event.key_code = static_cast<KeyCode>(keycode);
		event::notify(type_event);
	});

	glfwSetMouseButtonCallback(_window, [](GLFWwindow* window, int button, int action, int mods) {
		switch (action) {
			case GLFW_PRESS: {
				MousePressEvent mouse_event{};
				mouse_event.button_code = static_cast<MouseButton>(button);
				event::notify(mouse_event);
				break;
			}
			case GLFW_RELEASE: {
				MouseReleaseEvent mouse_event{};
				mouse_event.button_code = static_cast<MouseButton>(button);
				event::notify(mouse_event);
				break;
			}
			default: {
				break;
			}
		}
	});

	glfwSetCursorPosCallback(
			_window, [](GLFWwindow* window, const double x_pos, const double y_pos) {
				MouseMoveEvent move_event{};
				move_event.position = { static_cast<float>(x_pos), static_cast<float>(y_pos) };
				event::notify(move_event);
			});

	glfwSetScrollCallback(
			_window, [](GLFWwindow* window, const double x_offset, const double y_offset) {
				MouseScrollEvent scroll_event{};
				scroll_event.offset = { static_cast<float>(x_offset),
					static_cast<float>(y_offset) };
				event::notify(scroll_event);
			});
}

} //namespace gl