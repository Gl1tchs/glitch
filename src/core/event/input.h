#pragma once

#include "core/event/key_code.h"
#include "core/event/mouse_button.h"

class GL_API Input {
public:
	static void init();

	static bool is_key_pressed_once(KeyCode p_key);

	static bool is_key_pressed(KeyCode p_key);

	static bool is_key_released(KeyCode p_key);

	static bool is_mouse_pressed(MouseButton p_button);

	static bool is_mouse_released(MouseButton p_button);

	static Vec2f get_mouse_position();

	static Vec2f get_scroll_offset();
};
