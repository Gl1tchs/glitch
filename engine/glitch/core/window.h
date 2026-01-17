/**
 * @file window.h
 */

#pragma once

#include "glitch/core/defines.h"

#include <glgpu/glgpu.h>

#include <cstdint>
#include <string_view>

struct GLFWwindow;

namespace gl {

typedef GLFWwindow WindowHandle;

enum WindowCursorMode : int {
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
	Window(WindowCreateInfo info);
	~Window();

	void poll_events() const;

	bool is_open() const;

	Vec2u get_size() const;

	float get_aspect_ratio() const;

	void set_title(std::string_view title);

	WindowCursorMode get_cursor_mode() const;
	void set_cursor_mode(WindowCursorMode mode);

	WindowHandle* get_native_window();

private:
	void _assign_event_delegates();

private:
	WindowHandle* _window;
	WindowCursorMode _cursor_mode = WINDOW_CURSOR_MODE_NORMAL;
};

} //namespace gl