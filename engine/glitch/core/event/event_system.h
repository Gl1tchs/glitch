#pragma once

#include "glitch/core/core.h"
#include "glitch/core/event/key_code.h"
#include "glitch/core/event/mouse_button.h"

#include <glgpu/glgpu.h>

#include <functional>
#include <vector>

namespace gl {

struct KeyPressEvent {
	KeyCode key_code;
};

struct KeyReleaseEvent {
	KeyCode key_code;
};

struct KeyTypeEvent {
	KeyCode key_code;
};

struct MouseMoveEvent {
	Vec2f position;
};

struct MouseScrollEvent {
	Vec2f offset;
};

struct MousePressEvent {
	MouseButton button_code;
};

struct MouseReleaseEvent {
	MouseButton button_code;
};

struct WindowResizeEvent {
	Vec2u size;
};

struct WindowCloseEvent {};

template <typename T> using EventCallbackFunc = std::function<void(const T&)>;

namespace event {

template <typename T> inline auto g_callbacks = std::vector<EventCallbackFunc<T>>();

template <typename T> inline void subscribe(const EventCallbackFunc<T>& callback) {
	g_callbacks<T>.push_back(callback);
}

template <typename T> inline void unsubscribe() { g_callbacks<T>.clear(); }

template <typename T> inline void pop() { g_callbacks<T>.pop_back(); }

template <typename T> inline void notify(T& event) {
	for (const auto& callback : g_callbacks<T>) {
		callback(event);
	}
}

} // namespace event

} //namespace gl
