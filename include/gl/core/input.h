#pragma once

#include "gl/core/key_code.h"
#include "gl/core/mouse_code.h"

struct Input {
	static void init();

	static bool is_key_pressed_once(KeyCode key);

	static bool is_key_pressed(KeyCode key);

	static bool is_key_released(KeyCode key);

	static bool is_mouse_pressed(MouseCode button);

	static bool is_mouse_released(MouseCode button);

	static Vec2f get_mouse_position();

	static Vec2f get_scroll_offset();
};
