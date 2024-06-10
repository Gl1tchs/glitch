#include "testbed.h"

#include <core/event/input.h>
#include <renderer/mesh.h>
#include <renderer/render_backend.h>
#include <renderer/renderer.h>
#include <renderer/types.h>

TestBedApplication::TestBedApplication(const ApplicationCreateInfo& info) :
		Application(info) {}

void TestBedApplication::_on_start() {
	camera = create_ref<PerspectiveCameraNode>();

	get_window()->set_cursor_mode(WindowCursorMode::DISABLED);
	camera_controller.set_camera(camera.get());

	get_renderer()->get_scene_graph().push_node(camera);

	material = Material::create();

	scene = Mesh::load("assets/DamagedHelmet.glb", material);
	if (scene) {
		for (auto& child : scene->children) {
			child->transform.local_position.y = 0.75f;
			child->transform.local_rotation.x = 90.0f;
		}
		get_renderer()->get_scene_graph().push_node(scene);
	}

	Ref<RenderBackend> backend = get_renderer()->get_backend();

	SpirvData vertex_data = {};
	vertex_data.stage = SHADER_STAGE_VERTEX_BIT;
	vertex_data.byte_code = get_bundled_spirv_data("xy_plane.vert.spv");

	SpirvData fragment_data = {};
	fragment_data.stage = SHADER_STAGE_FRAGMENT_BIT;
	fragment_data.byte_code = get_bundled_spirv_data("xy_plane.frag.spv");

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
			DATA_FORMAT_R16G16B16A16_SFLOAT);
	rendering_state.depth_attachment = DATA_FORMAT_D32_SFLOAT;

	grid_pipeline = backend->render_pipeline_create(grid_shader,
			RENDER_PRIMITIVE_TRIANGLES, rasterization, multisample,
			depth_stencil_state, color_blend_state, 0, rendering_state);
}

void TestBedApplication::_on_update(float dt) {
	camera->aspect_ratio = get_window()->get_aspect_ratio();
	camera_controller.update(dt);

	get_renderer()->submit(RENDER_STATE_GEOMETRY,
			[this](Ref<RenderBackend> backend, CommandBuffer cmd,
					DeletionQueue& frame_deletion) {
				Buffer camera_uniform_buffer =
						backend->buffer_create(sizeof(GridCameraUniform),
								BUFFER_USAGE_UNIFORM_BUFFER_BIT |
										BUFFER_USAGE_TRANSFER_SRC_BIT,
								MEMORY_ALLOCATION_TYPE_CPU);

				frame_deletion.push_function(
						[=]() { backend->buffer_free(camera_uniform_buffer); });

				GridCameraUniform* camera_uniform_data =
						(GridCameraUniform*)backend->buffer_map(
								camera_uniform_buffer);
				{
					get_renderer()->get_scene_graph().traverse<CameraNode>(
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
				backend->buffer_unmap(camera_uniform_buffer);

				BoundUniform uniform;
				uniform.type = UNIFORM_TYPE_UNIFORM_BUFFER;
				uniform.binding = 0;
				uniform.ids.push_back(camera_uniform_buffer);

				UniformSet camera_uniform =
						backend->uniform_set_create(uniform, grid_shader, 0);
				frame_deletion.push_function(
						[=]() { backend->uniform_set_free(camera_uniform); });

				backend->command_bind_graphics_pipeline(cmd, grid_pipeline);

				backend->command_bind_uniform_sets(
						cmd, grid_shader, 0, camera_uniform);

				backend->command_draw(cmd, 6);
			});
}

void TestBedApplication::_on_destroy() {
	get_renderer()->wait_for_device();

	Ref<RenderBackend> backend = get_renderer()->get_backend();

	backend->pipeline_free(grid_pipeline);
	backend->shader_free(grid_shader);

	Material::destroy(material);
}
