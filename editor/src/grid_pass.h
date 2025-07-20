/**
 * @file grid_pass.h
 *
 */

#pragma once

#include "glitch/renderer/camera.h"
#include "glitch/renderer/graphics_pass.h"

using namespace gl;

class GridPass : public GraphicsPass {
public:
	GL_DEFINE_GRAPHICS_PASS("Grid Pass")

	struct PushConstants {
		glm::mat4 view_proj;
		glm::vec3 camera_pos;
		float grid_size;
	};

	virtual ~GridPass();

	void setup(Renderer& p_renderer) override;
	void execute(CommandBuffer p_cmd, Renderer& p_renderer) override;

	void set_camera(const PerspectiveCamera& p_camera);

private:
	Shader grid_shader;
	Pipeline grid_pipeline;

	PerspectiveCamera camera;
	PushConstants push_constants = {};
};