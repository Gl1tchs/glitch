#include "glitch/core/event/input.h"

#include "glitch/core/event/event_system.h"

#include <unordered_map>

namespace gl {

static std::unordered_map<KeyCode, bool> s_key_press_states = {};
static std::unordered_map<KeyCode, bool> s_key_release_states = {};
static std::unordered_map<KeyCode, bool> s_keys_held_states = {};

static std::unordered_map<MouseButton, bool> s_mouse_press_states = {};
static std::unordered_map<MouseButton, bool> s_mouse_release_states = {};
static Vec2f mouse_position = Vec2f(0.0f);
static Vec2f scroll_offset = Vec2f(0.0f);

void Input::init() {
	event::subscribe<KeyPressEvent>([&](const KeyPressEvent& event) {
		s_key_press_states[event.key_code] = true;
		s_key_release_states[event.key_code] = false;
	});

	event::subscribe<KeyReleaseEvent>([&](const KeyReleaseEvent& event) {
		// if key already held remove
		if (const auto it = s_keys_held_states.find(event.key_code);
				it != s_keys_held_states.end()) {
			s_keys_held_states[event.key_code] = false;
		}

		s_key_press_states[event.key_code] = false;
		s_key_release_states[event.key_code] = true;
	});

	event::subscribe<MousePressEvent>([&](const MousePressEvent& event) {
		s_mouse_press_states[event.button_code] = true;
		s_mouse_release_states[event.button_code] = false;
	});

	event::subscribe<MouseReleaseEvent>([&](const MouseReleaseEvent& event) {
		s_mouse_press_states[event.button_code] = false;
		s_mouse_release_states[event.button_code] = true;
	});

	event::subscribe<MouseMoveEvent>(
			[&](const MouseMoveEvent& event) { mouse_position = event.position; });

	event::subscribe<MouseScrollEvent>(
			[&](const MouseScrollEvent& event) { scroll_offset = event.offset; });
}

bool Input::is_key_pressed_once(KeyCode key) {
	// if key already held return
	if (const auto it = s_keys_held_states.find(key);
			it != s_keys_held_states.end() && s_keys_held_states[key]) {
		return false;
	}

	return is_key_pressed(key);
}

bool Input::is_key_pressed(KeyCode key) {
	const auto it = s_key_press_states.find(key);
	if (it == s_key_press_states.end()) {
		return false;
	}

	bool pressed = it->second;

	if (pressed) {
		// if key haven't held add
		if (const auto it = s_keys_held_states.find(key);
				it == s_keys_held_states.end() || // if doesn't exists
				(it != s_keys_held_states.end() && !s_keys_held_states[key])) //if exists but havent
																			  //pressed
		{
			s_keys_held_states[key] = true;
		}
	}

	return pressed;
}

bool Input::is_key_released(KeyCode key) {
	const auto it = s_key_release_states.find(key);
	if (it == s_key_release_states.end()) {
		return false;
	}

	return it->second;
}

bool Input::is_mouse_pressed(MouseButton button) {
	const auto it = s_mouse_press_states.find(button);
	if (it != s_mouse_press_states.end()) {
		return it->second;
	}
	return false;
}

bool Input::is_mouse_released(MouseButton button) {
	const auto it = s_mouse_release_states.find(button);
	if (it != s_mouse_release_states.end()) {
		return it->second;
	}
	return false;
}

Vec2f Input::get_mouse_position() { return mouse_position; }

Vec2f Input::get_scroll_offset() {
	const Vec2f old_scroll_offset = scroll_offset;
	scroll_offset = { 0, 0 };
	return old_scroll_offset;
}

} //namespace gl