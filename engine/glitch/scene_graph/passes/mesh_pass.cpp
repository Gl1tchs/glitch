#include "glitch/scene_graph/passes/mesh_pass.h"

#include "glitch/core/application.h"

namespace gl {

void MeshPass::setup(Renderer& p_renderer) {
	GL_PROFILE_SCOPE;

	scene_data_sbo = StorageBuffer::create(sizeof(SceneData), &scene_data);
	push_constants.scene_buffer = scene_data_sbo->get_device_address();
}

void MeshPass::execute(CommandBuffer p_cmd, Renderer& p_renderer) {
	GL_PROFILE_SCOPE;

	if (!graph) {
		GL_LOG_ERROR("[MeshPass::execute] No scene graph bound to render");
		return;
	}

	Ref<RenderBackend> backend = p_renderer.get_backend();

	camera.aspect_ratio =
			Application::get_instance()->get_window()->get_aspect_ratio();

	scene_data.view_projection =
			camera.get_projection_matrix() * camera.get_view_matrix();
	scene_data.camera_position = camera.transform.position;

	// Reupload the scene buffer if it's updated
	size_t hash = hash64(scene_data.view_projection);
	hash_combine(hash, hash64(scene_data.camera_position));

	if (scene_data_hash != hash) {
		scene_data_sbo->upload(&scene_data);
		scene_data_hash = hash;
	}

	// TODO: dirty flags to cache
	graph->update_transforms();

	// Construct a frustum culled render queue to render only visible primitives
	Frustum view_frustum = Frustum::from_view_proj(scene_data.view_projection);
	RenderQueue render_queue = graph->construct_render_queue(view_frustum);

	// Multilevel sorting by used pipeline and z index
	render_queue.sort();

	p_renderer.begin_rendering(p_cmd,
			p_renderer.get_render_image("geo_albedo").value(),
			p_renderer.get_render_image("geo_depth").value());

	// TODO: preprocess and do an instanced rendering and ibl

	Pipeline bound_pipeline = GL_NULL_HANDLE;
	for (const RenderObject& renderable : render_queue) {
		Ref<MeshPrimitive> primitive = renderable.primitive;

		// Bind the pipeline if not already bound
		Pipeline pipeline = primitive->material->definition->pipeline;
		if (pipeline != bound_pipeline) {
			backend->command_bind_graphics_pipeline(p_cmd, pipeline);
			bound_pipeline = pipeline;
		}

		// set = 0 material data
		primitive->material->bind_uniform_set(p_cmd);

		// Push constants
		{
			push_constants.vertex_buffer = primitive->vertex_buffer_address;

			// Object transformation
			push_constants.transform = renderable.transform;

			backend->command_push_constants(p_cmd,
					primitive->material->definition->shader, 0,
					sizeof(PushConstants), &push_constants);
		}

		// Render
		backend->command_bind_index_buffer(
				p_cmd, primitive->index_buffer, 0, IndexType::UINT32);

		backend->command_draw_indexed(p_cmd, primitive->index_count);

		{
			ApplicationPerfStats& stats =
					Application::get_instance()->get_perf_stats();

			stats.renderer_stats.draw_calls++;
			stats.renderer_stats.index_count += primitive->index_count;
		}
	}

	p_renderer.end_rendering(p_cmd);
}

void MeshPass::set_camera(const PerspectiveCamera& p_camera) {
	camera = p_camera;
}

void MeshPass::set_scene_graph(SceneGraph* p_graph) { graph = p_graph; }

} //namespace gl
