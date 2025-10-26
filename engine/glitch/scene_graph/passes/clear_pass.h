/**
 * @file clear_pass.h
 *
 */

#pragma once

#include "glitch/renderer/graphics_pass.h"

namespace gl {

/**
 * Graphic pass that's goal to initialize render targets and clear the screen
 * NOTE: Priority of the clear pass in renderer is -10
 */
class ClearPass : public GraphicsPass {
public:
	GL_DEFINE_GRAPHICS_PASS("Clear Pass")

	virtual ~ClearPass() = default;

	void setup(Renderer& p_renderer) override;
	void execute(CommandBuffer p_cmd, Renderer& p_renderer) override;
};

} //namespace gl