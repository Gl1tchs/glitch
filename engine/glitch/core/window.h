/**
 * @file window.h
 */

#pragma once

struct GLFWwindow;

namespace gl {

typedef GLFWwindow WindowHandle;

enum WindowCursorMode {
	WINDOW_CURSOR_MODE_NORMAL = 0,
	WINDOW_CURSOR_MODE_HIDDEN = 1,
	WINDOW_CURSOR_MODE_DISABLED = 2,
	WINDOW_CURSOR_MODE_CAPTURED = 3,
};

struct WindowCreateInfo {
	const char* title = "glitch";
	uint32_t w = 1280;
	uint32_t h = 768;
};

class GL_API Window {
public:
	Window(WindowCreateInfo p_info);
	~Window();

	void poll_events() const;

	bool is_open() const;

	glm::uvec2 get_size() const;

	float get_aspect_ratio() const;

	void set_title(std::string_view p_title);

	WindowCursorMode get_cursor_mode() const;
	void set_cursor_mode(WindowCursorMode p_mode);

	WindowHandle* get_native_window();

private:
	void _assign_event_delegates();

private:
	WindowHandle* window;

	WindowCursorMode cursor_mode = WINDOW_CURSOR_MODE_NORMAL;
};

} //namespace gl