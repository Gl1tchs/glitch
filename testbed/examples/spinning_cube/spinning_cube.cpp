#include "examples/spinning_cube/spinning_cube.h"

#include <glitch/renderer/render_backend.h>
#include <glitch/renderer/renderer.h>
#include <glitch/renderer/types.h>
#include <imgui/imgui.h>


#include "shader_utils.h"

struct Vertex {
	glm::vec3 position;
};

struct PushConstants {
	glm::mat4 view_proj;
	glm::mat4 transform;
};

void ExampleSpinningCube::on_init() {
	backend = Renderer::get_backend();
	SpirvData vertex_data = {};
	vertex_data.stage = SHADER_STAGE_VERTEX_BIT;
	vertex_data.byte_code =
			get_spirv_data("shaders/spinning_cube/cube.vert.spv");

	SpirvData fragment_data = {};
	fragment_data.stage = SHADER_STAGE_FRAGMENT_BIT;
	fragment_data.byte_code =
			get_spirv_data("shaders/spinning_cube/cube.frag.spv");

	const std::vector<SpirvData> shader_stages = {
		vertex_data,
		fragment_data,
	};

	shader = backend->shader_create_from_bytecode(shader_stages);

	PipelineVertexInputState vertex_input = {};
	vertex_input.stride = sizeof(Vertex);

	PipelineDepthStencilState depth_stencil_state = {};
	depth_stencil_state.enable_depth_test = true;
	depth_stencil_state.enable_depth_write = true;

	PipelineColorBlendState color_blend_state =
			PipelineColorBlendState::create_disabled();

	RenderingState rendering_state = {};
	rendering_state.color_attachments.push_back(
			Renderer::get_draw_image_format());
	rendering_state.depth_attachment = Renderer::get_depth_image_format();

	pipeline = backend->render_pipeline_create(shader,
			RENDER_PRIMITIVE_TRIANGLES, vertex_input, {}, {},
			depth_stencil_state, color_blend_state, 0, rendering_state);

	const std::vector<Vertex> vertices = {
		{ { -1.0f, -1.0f, 1.0f } }, //0
		{ { 1.0f, -1.0f, 1.0f } }, //1
		{ { -1.0f, 1.0f, 1.0f } }, //2
		{ { 1.0f, 1.0f, 1.0f } }, //3
		{ { -1.0f, -1.0f, -1.0f } }, //4
		{ { 1.0f, -1.0f, -1.0f } }, //5
		{ { -1.0f, 1.0f, -1.0f } }, //6
		{ { 1.0f, 1.0f, -1.0f } }, //7
	};

	vertex_buffer = backend->buffer_create(vertices.size() * sizeof(Vertex),
			BUFFER_USAGE_VERTEX_BUFFER_BIT | BUFFER_USAGE_TRANSFER_DST_BIT,
			MEMORY_ALLOCATION_TYPE_CPU);

	void* mapped_data = backend->buffer_map(vertex_buffer);
	memcpy(mapped_data, vertices.data(), vertices.size() * sizeof(Vertex));
	backend->buffer_unmap(vertex_buffer);

	const std::vector<uint32_t> indices{ //Top
		2, 6, 7, 2, 3, 7,
		//Bottom
		0, 4, 5, 0, 1, 5,
		//Left
		0, 2, 6, 0, 4, 6,
		//Right
		1, 3, 7, 1, 5, 7,
		//Front
		0, 2, 3, 0, 1, 3,
		//Back
		4, 6, 7, 4, 5, 7
	};

	index_buffer = backend->buffer_create(indices.size() * sizeof(uint32_t),
			BUFFER_USAGE_INDEX_BUFFER_BIT | BUFFER_USAGE_TRANSFER_DST_BIT,
			MEMORY_ALLOCATION_TYPE_CPU);

	mapped_data = backend->buffer_map(index_buffer);
	memcpy(mapped_data, indices.data(), indices.size() * sizeof(uint32_t));
	backend->buffer_unmap(index_buffer);
}

void ExampleSpinningCube::on_render(
		CommandBuffer p_cmd, const SceneData& p_scene_data) {
	GL_PROFILE_SCOPE;

	// rotate the cube on y axis
	cube_transform.local_rotation.y += 10.0f * p_scene_data.delta_time;

	backend->command_bind_graphics_pipeline(p_cmd, pipeline);

	// Push constants
	PushConstants push_constants = {};
	push_constants.view_proj = p_scene_data.projection * p_scene_data.view;
	push_constants.transform = cube_transform.get_transform_matrix();

	backend->command_push_constants(
			p_cmd, shader, 0, sizeof(PushConstants), &push_constants);

	backend->command_bind_vertex_buffers(p_cmd, 0, { vertex_buffer }, { 0 });
	backend->command_bind_index_buffer(
			p_cmd, index_buffer, 0, INDEX_TYPE_UINT32);

	backend->command_draw_indexed(p_cmd, 36);
}

void ExampleSpinningCube::on_destroy() {
	backend->pipeline_free(pipeline);
	backend->shader_free(shader);
	backend->buffer_free(vertex_buffer);
	backend->buffer_free(index_buffer);
}

void ExampleSpinningCube::on_imgui() { ImGui::ShowDemoWindow(); }
