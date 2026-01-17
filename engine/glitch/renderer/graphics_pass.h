/**
 * @file graphics_pass.h
 *
 */

#pragma once

#include "glitch/renderer/renderer.h"

#define GL_DEFINE_GRAPHICS_PASS(name)                                                              \
	const char* get_name() const override { return name; };

namespace gl {

class GraphicsPass {
public:
	virtual ~GraphicsPass() = default;

	virtual void setup(Renderer& renderer) = 0;
	virtual void execute(CommandBuffer cmd, Renderer& renderer) = 0;

	virtual const char* get_name() const = 0;

	bool is_active() const { return _active; }
	void set_active(bool active) { _active = active; }

private:
	bool _active = true;
};

} //namespace gl
