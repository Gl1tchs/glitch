/**
 * @file input.h
 */

#pragma once

#include "glitch/core/defines.h"
#include "glitch/core/event/key_code.h"
#include "glitch/core/event/mouse_button.h"

#include <glgpu/glgpu.h>

namespace gl {

class GL_API Input {
public:
	static void init();

	static bool is_key_pressed_once(KeyCode key);

	static bool is_key_pressed(KeyCode key);

	static bool is_key_released(KeyCode key);

	static bool is_mouse_pressed(MouseButton button);

	static bool is_mouse_released(MouseButton button);

	static Vec2f get_mouse_position();

	static Vec2f get_scroll_offset();
};

} //namespace gl