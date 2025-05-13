#pragma once

#include <glitch/renderer/types.h>

#include "scene_data.h"

#define IMPL_EXAMPLE(name)                                                     \
	const char* get_name() const override { return name; }

class Example {
public:
	virtual const char* get_name() const = 0;

	virtual void on_init() = 0;
	virtual void on_render(
			CommandBuffer p_cmd, const SceneData& p_scene_data) = 0;
	virtual void on_destroy() = 0;

	virtual void on_imgui() {};
};
