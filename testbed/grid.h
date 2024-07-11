#pragma once

#include <renderer/renderer.h>

class Grid {
public:
	Grid();
	~Grid();

	void render(Ref<Renderer> p_renderer, Scene* p_scene);

private:
	Pipeline grid_pipeline;
	Shader grid_shader;

	struct GridCameraUniform {
		glm::mat4 view;
		glm::mat4 proj;
		float near_plane;
		float far_plane;
	};
};
