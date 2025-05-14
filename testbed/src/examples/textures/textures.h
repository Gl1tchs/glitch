#pragma once

#include "example.h"

#include <glitch/core/transform.h>
#include <glitch/renderer/render_backend.h>
#include <glitch/renderer/types.h>

class ExampleTextures : public Example {
public:
	IMPL_EXAMPLE("Textures")

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

	UniformSet uniform_set;

	Image texture;
	Sampler sampler;
};
