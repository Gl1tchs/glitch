#include "examples/hello_triangle/hello_triangle.h"

#include <glitch/core/debug/log.h>
#include <glitch/renderer/render_backend.h>
#include <glitch/renderer/renderer.h>
#include <glitch/renderer/types.h>

#include "shader_utils.h"

struct Vertex {
	glm::vec3 position;
	glm::vec3 color;
};

void ExampleHelloTriangle::on_init() {
	backend = Renderer::get_backend();

	SpirvData vertex_data = {};
	vertex_data.stage = SHADER_STAGE_VERTEX_BIT;
	vertex_data.byte_code =
			get_spirv_data("shaders/hello_triangle/triangle.vert.spv");

	SpirvData fragment_data = {};
	fragment_data.stage = SHADER_STAGE_FRAGMENT_BIT;
	fragment_data.byte_code =
			get_spirv_data("shaders/hello_triangle/triangle.frag.spv");

	std::vector<SpirvData> shader_stages = {
		vertex_data,
		fragment_data,
	};

	shader = backend->shader_create_from_bytecode(shader_stages);

	// Pipeline state
	PipelineVertexInputState vertex_input_state = {};
	vertex_input_state.stride = sizeof(Vertex);

	PipelineDepthStencilState depth_stencil_state = {};
	depth_stencil_state.enable_depth_test =
			false; // No depth testing for a 2D triangle
	depth_stencil_state.enable_depth_write = false;

	PipelineColorBlendState color_blend_state =
			PipelineColorBlendState::create_disabled();

	RenderingState rendering_state = {};
	rendering_state.color_attachments.push_back(
			Renderer::get_draw_image_format());

	pipeline = backend->render_pipeline_create(shader,
			RENDER_PRIMITIVE_TRIANGLES, vertex_input_state, {}, {},
			depth_stencil_state, color_blend_state, 0, rendering_state);

	// Vertex buffer data
	const std::vector<Vertex> vertices = {
		{
				{ 0.0f, -0.5f, 0.0f },
				{ 1.0f, 0.0f, 0.0f },
		}, // Bottom vertex (Red)
		{
				{ -0.5f, 0.5f, 0.0f },
				{ 0.0f, 1.0f, 0.0f },
		}, // Top-left vertex (Green)
		{
				{ 0.5f, 0.5f, 0.0f },
				{ 0.0f, 0.0f, 1.0f },
		}, // Top-right vertex (Blue)
	};

	vertex_buffer = backend->buffer_create(vertices.size() * sizeof(Vertex),
			BUFFER_USAGE_VERTEX_BUFFER_BIT | BUFFER_USAGE_TRANSFER_DST_BIT,
			MEMORY_ALLOCATION_TYPE_CPU);

	void* mapped_data = backend->buffer_map(vertex_buffer);
	memcpy(mapped_data, vertices.data(), vertices.size() * sizeof(Vertex));
	backend->buffer_unmap(vertex_buffer);
};

void ExampleHelloTriangle::on_render(
		CommandBuffer p_cmd, const ExampleSceneData& p_scene_data) {
	GL_PROFILE_SCOPE;

	backend->command_bind_graphics_pipeline(p_cmd, pipeline);

	backend->command_bind_vertex_buffers(p_cmd, 0, { vertex_buffer }, { 0 });

	backend->command_draw(p_cmd, 3, 1, 0, 0);
}

void ExampleHelloTriangle::on_destroy() {
	backend->pipeline_free(pipeline);
	backend->shader_free(shader);
	backend->buffer_free(vertex_buffer);
}
