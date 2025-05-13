/**
 * @file input.h
 */

#pragma once

#include "glitch/core/event/key_code.h"
#include "glitch/core/event/mouse_button.h"

class GL_API Input {
public:
	static void init();

	static bool is_key_pressed_once(KeyCode p_key);

	static bool is_key_pressed(KeyCode p_key);

	static bool is_key_released(KeyCode p_key);

	static bool is_mouse_pressed(MouseButton p_button);

	static bool is_mouse_released(MouseButton p_button);

	static glm::vec2 get_mouse_position();

	static glm::vec2 get_scroll_offset();
};
