#pragma once

#include "core/event/key_code.h"
#include "core/event/mouse_button.h"

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
	Vec2i size;
};

struct WindowCloseEvent {};

template <typename T> using EventCallbackFunc = std::function<void(const T&)>;

namespace event {

template <typename T>
inline auto g_callbacks = std::vector<EventCallbackFunc<T>>();

template <typename T>
inline void subscribe(const EventCallbackFunc<T>& p_callback) {
	g_callbacks<T>.push_back(p_callback);
}

template <typename T> inline void unsubscribe() { g_callbacks<T>.clear(); }

template <typename T> inline void pop() { g_callbacks<T>.pop_back(); }

template <typename T> inline void notify(T& p_event) {
	for (auto& callback : g_callbacks<T>) {
		callback(p_event);
	}
}

} // namespace event
