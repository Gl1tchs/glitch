#include "examples/textures/textures.h"

#include <glitch/renderer/render_backend.h>
#include <glitch/renderer/renderer.h>
#include <glitch/renderer/types.h>
#include <imgui/imgui.h>


#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include "shader_utils.h"

struct Vertex {
	glm::vec3 position;
	glm::vec2 uv;
};

void ExampleTextures::on_init() {
	backend = Renderer::get_backend();
	SpirvData vertex_data = {};
	vertex_data.stage = SHADER_STAGE_VERTEX_BIT;
	vertex_data.byte_code =
			get_spirv_data("shaders/textures/textured_rect.vert.spv");

	SpirvData fragment_data = {};
	fragment_data.stage = SHADER_STAGE_FRAGMENT_BIT;
	fragment_data.byte_code =
			get_spirv_data("shaders/textures/textured_rect.frag.spv");

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
		{ { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f } },
		{ { -0.5f, 0.5f, 0.0f }, { 0.0f, 1.0f } },
		{ { 0.5f, 0.5f, 0.0f }, { 1.0f, 1.0f } },
		{ { 0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f } },
	};

	vertex_buffer = backend->buffer_create(vertices.size() * sizeof(Vertex),
			BUFFER_USAGE_VERTEX_BUFFER_BIT | BUFFER_USAGE_TRANSFER_DST_BIT,
			MEMORY_ALLOCATION_TYPE_CPU);

	void* mapped_data = backend->buffer_map(vertex_buffer);
	memcpy(mapped_data, vertices.data(), vertices.size() * sizeof(Vertex));
	backend->buffer_unmap(vertex_buffer);

	const std::vector<uint32_t> indices{ 0, 1, 2, 2, 3, 0 };

	index_buffer = backend->buffer_create(indices.size() * sizeof(uint32_t),
			BUFFER_USAGE_INDEX_BUFFER_BIT | BUFFER_USAGE_TRANSFER_DST_BIT,
			MEMORY_ALLOCATION_TYPE_CPU);

	mapped_data = backend->buffer_map(index_buffer);
	memcpy(mapped_data, indices.data(), indices.size() * sizeof(uint32_t));
	backend->buffer_unmap(index_buffer);

	uint32_t x, y;
	stbi_uc* image_data = stbi_load("assets/chill_guy.png", (int*)&x, (int*)&y,
			nullptr, STBI_rgb_alpha);

	texture = backend->image_create(
			DATA_FORMAT_R8G8B8A8_UNORM, { x, y }, image_data);

	stbi_image_free(image_data);

	sampler = backend->sampler_create();

	ShaderUniform uniform;
	uniform.type = UNIFORM_TYPE_SAMPLER_WITH_TEXTURE;
	uniform.binding = 0;
	uniform.data.push_back(sampler);
	uniform.data.push_back(texture);

	uniform_set = backend->uniform_set_create(uniform, shader, 0);
}

void ExampleTextures::on_render(
		CommandBuffer p_cmd, const SceneData& p_scene_data) {
	GL_PROFILE_SCOPE;

	backend->command_bind_graphics_pipeline(p_cmd, pipeline);

	// Bind uniforms
	backend->command_bind_uniform_sets(p_cmd, shader, 0, uniform_set);

	backend->command_bind_vertex_buffers(p_cmd, 0, { vertex_buffer }, { 0 });
	backend->command_bind_index_buffer(
			p_cmd, index_buffer, 0, INDEX_TYPE_UINT32);

	backend->command_draw_indexed(p_cmd, 6);
}

void ExampleTextures::on_destroy() {
	backend->pipeline_free(pipeline);
	backend->shader_free(shader);
	backend->buffer_free(vertex_buffer);
	backend->buffer_free(index_buffer);
	backend->uniform_set_free(uniform_set);
	backend->image_free(texture);
	backend->sampler_free(sampler);
}

void ExampleTextures::on_imgui() {}
