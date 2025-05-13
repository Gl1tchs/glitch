#pragma once

#include "example.h"

#include <glitch/core/transform.h>
#include <glitch/renderer/render_backend.h>
#include <glitch/renderer/types.h>

class ExampleSpinningCube : public Example {
public:
	IMPL_EXAMPLE("Spinning Cube")

	void on_init() override;
	void on_render(CommandBuffer p_cmd, const SceneData& p_scene_data) override;
	void on_destroy() override;

	void on_imgui() override;

private:
	Ref<RenderBackend> backend;

	Pipeline pipeline;
	Shader shader;

	Buffer vertex_buffer;
	Buffer index_buffer;

	Transform cube_transform;
};
