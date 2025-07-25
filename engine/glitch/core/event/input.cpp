#include "glitch/core/event/input.h"

#include "glitch/core/event/event_system.h"

namespace gl {

inline static std::unordered_map<KeyCode, bool> key_press_states = {};
inline static std::unordered_map<KeyCode, bool> key_release_states = {};
inline static std::unordered_map<KeyCode, bool> keys_held_states = {};

inline static std::unordered_map<MouseButton, bool> mouse_press_states = {};
inline static std::unordered_map<MouseButton, bool> mouse_release_states = {};
inline static glm::vec2 mouse_position = glm::vec2(0.0f);
inline static glm::vec2 scroll_offset = glm::vec2(0.0f);

void Input::init() {
	event::subscribe<KeyPressEvent>([&](const KeyPressEvent& event) {
		key_press_states[event.key_code] = true;
		key_release_states[event.key_code] = false;
	});

	event::subscribe<KeyReleaseEvent>([&](const KeyReleaseEvent& event) {
		// if key already held remove
		if (const auto it = keys_held_states.find(event.key_code);
				it != keys_held_states.end()) {
			keys_held_states[event.key_code] = false;
		}

		key_press_states[event.key_code] = false;
		key_release_states[event.key_code] = true;
	});

	event::subscribe<MousePressEvent>([&](const MousePressEvent& event) {
		mouse_press_states[event.button_code] = true;
		mouse_release_states[event.button_code] = false;
	});

	event::subscribe<MouseReleaseEvent>([&](const MouseReleaseEvent& event) {
		mouse_press_states[event.button_code] = false;
		mouse_release_states[event.button_code] = true;
	});

	event::subscribe<MouseMoveEvent>([&](const MouseMoveEvent& event) {
		mouse_position = event.position;
	});

	event::subscribe<MouseScrollEvent>([&](const MouseScrollEvent& event) {
		scroll_offset = event.offset;
	});
}

bool Input::is_key_pressed_once(KeyCode p_key) {
	// if key already held return
	if (const auto it = keys_held_states.find(p_key);
			it != keys_held_states.end() && keys_held_states[p_key]) {
		return false;
	}

	return is_key_pressed(p_key);
}

bool Input::is_key_pressed(KeyCode p_key) {
	const auto it = key_press_states.find(p_key);
	if (it == key_press_states.end()) {
		return false;
	}

	bool pressed = it->second;

	if (pressed) {
		// if key haven't held add
		if (const auto it = keys_held_states.find(p_key);
				it == keys_held_states.end() || // if doesn't exists
				(it != keys_held_states.end() &&
						!keys_held_states[p_key])) //if exists but havent
												   //pressed
		{
			keys_held_states[p_key] = true;
		}
	}

	return pressed;
}

bool Input::is_key_released(KeyCode p_key) {
	const auto it = key_release_states.find(p_key);
	if (it == key_release_states.end()) {
		return false;
	}

	return it->second;
}

bool Input::is_mouse_pressed(MouseButton p_button) {
	const auto it = mouse_press_states.find(p_button);
	if (it != mouse_press_states.end()) {
		return it->second;
	}
	return false;
}

bool Input::is_mouse_released(MouseButton p_button) {
	const auto it = mouse_release_states.find(p_button);
	if (it != mouse_release_states.end()) {
		return it->second;
	}
	return false;
}

glm::vec2 Input::get_mouse_position() { return mouse_position; }

glm::vec2 Input::get_scroll_offset() {
	const glm::vec2 old_scroll_offset = scroll_offset;
	scroll_offset = { 0, 0 };
	return old_scroll_offset;
}

} //namespace gl