#pragma once

#include <renderer/renderer.h>

class Grid {
public:
	Grid(Ref<Renderer> p_renderer);
	~Grid();

	void render();

private:
	Ref<Renderer> renderer;

	Pipeline grid_pipeline;
	Shader grid_shader;

	struct GridCameraUniform {
		glm::mat4 view;
		glm::mat4 proj;
		float near_plane;
		float far_plane;
	};
};
