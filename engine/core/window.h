#pragma once

struct GLFWwindow;

enum class WindowCursorMode {
	NORMAL = 0,
	HIDDEN = 1,
	DISABLED = 2,
	CAPTURED = 3,
};

struct WindowCreateInfo {
	const char* title = "eve game";
	uint32_t w = 1280;
	uint32_t h = 768;
};

class Window {
public:
	Window(WindowCreateInfo info);
	~Window();

	void poll_events() const;

	bool is_open() const;

	Vector2u get_size() const;

	float get_aspect_ratio() const;

	void set_title(std::string_view title);

	WindowCursorMode get_cursor_mode() const;
	void set_cursor_mode(WindowCursorMode mode);

	GLFWwindow* get_native_window();

private:
	void _assign_event_delegates();

private:
	GLFWwindow* window;

	WindowCursorMode cursor_mode = WindowCursorMode::NORMAL;
};
