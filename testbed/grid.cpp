#include "grid.h"

#include <renderer/camera.h>
#include <renderer/render_backend.h>

static std::vector<uint32_t> _get_spirv_data(const fs::path& p_filepath) {
	size_t file_size = fs::file_size(p_filepath);

	std::ifstream file(p_filepath, std::ios::in | std::ios::binary);

	if (!file.is_open()) {
		return {};
	}

	std::vector<uint32_t> buffer(file_size);
	file.read(reinterpret_cast<char*>(buffer.data()), file_size);

	return buffer;
}

Grid::Grid(Ref<Renderer> p_renderer) : renderer(p_renderer) {
	Ref<RenderBackend> backend = renderer->get_backend();

	SpirvData vertex_data = {};
	vertex_data.stage = SHADER_STAGE_VERTEX_BIT;
	vertex_data.byte_code = _get_spirv_data("assets/shaders/xy_plane.vert.spv");

	SpirvData fragment_data = {};
	fragment_data.stage = SHADER_STAGE_FRAGMENT_BIT;
	fragment_data.byte_code =
			_get_spirv_data("assets/shaders/xy_plane.frag.spv");

	std::vector<SpirvData> shader_stages = {
		vertex_data,
		fragment_data,
	};

	grid_shader = backend->shader_create_from_bytecode(shader_stages);

	PipelineRasterizationState rasterization = {};

	PipelineMultisampleState multisample = {};

	PipelineDepthStencilState depth_stencil_state = {};
	depth_stencil_state.depth_compare_operator = COMPARE_OP_LESS;
	depth_stencil_state.enable_depth_test = true;
	depth_stencil_state.enable_depth_write = true;
	depth_stencil_state.enable_depth_range = true;

	PipelineColorBlendState color_blend_state =
			PipelineColorBlendState::create_blend();

	RenderingState rendering_state = {};
	rendering_state.color_attachments.push_back(
			renderer->get_draw_image_format());
	rendering_state.depth_attachment = renderer->get_depth_image_format();

	grid_pipeline = backend->render_pipeline_create(grid_shader,
			RENDER_PRIMITIVE_TRIANGLES, rasterization, multisample,
			depth_stencil_state, color_blend_state, 0, rendering_state);
}

Grid::~Grid() {
	renderer->wait_for_device();

	Ref<RenderBackend> backend = renderer->get_backend();

	backend->pipeline_free(grid_pipeline);
	backend->shader_free(grid_shader);
}

void Grid::render() {
	renderer->submit(RENDER_STATE_GEOMETRY,
			[this](Ref<RenderBackend> p_backend, CommandBuffer p_cmd,
					Image p_draw_image, DeletionQueue& p_frame_deletion) {
				Buffer camera_uniform_buffer =
						p_backend->buffer_create(sizeof(GridCameraUniform),
								BUFFER_USAGE_UNIFORM_BUFFER_BIT |
										BUFFER_USAGE_TRANSFER_SRC_BIT,
								MEMORY_ALLOCATION_TYPE_CPU);

				p_frame_deletion.push_function([=]() {
					p_backend->buffer_free(camera_uniform_buffer);
				});

				GridCameraUniform* camera_uniform_data =
						(GridCameraUniform*)p_backend->buffer_map(
								camera_uniform_buffer);
				{
					renderer->get_scene_graph().traverse<CameraNode>(
							[camera_uniform_data](CameraNode* camera) {
								camera_uniform_data->view =
										camera->get_view_matrix();
								camera_uniform_data->proj =
										camera->get_projection_matrix();
								camera_uniform_data->near_plane =
										camera->near_clip;
								camera_uniform_data->far_plane =
										camera->far_clip;
								return true;
							});
				}
				p_backend->buffer_unmap(camera_uniform_buffer);

				ShaderUniform uniform;
				uniform.type = UNIFORM_TYPE_UNIFORM_BUFFER;
				uniform.binding = 0;
				uniform.data.push_back(camera_uniform_buffer);

				UniformSet camera_uniform =
						p_backend->uniform_set_create(uniform, grid_shader, 0);
				p_frame_deletion.push_function(
						[=]() { p_backend->uniform_set_free(camera_uniform); });

				p_backend->command_bind_graphics_pipeline(p_cmd, grid_pipeline);

				p_backend->command_bind_uniform_sets(
						p_cmd, grid_shader, 0, camera_uniform);

				p_backend->command_draw(p_cmd, 6);
			});
}
