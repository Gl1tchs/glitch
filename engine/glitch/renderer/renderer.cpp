#include "glitch/renderer/renderer.h"

#include "glitch/core/application.h"
#include "glitch/core/transform.h"
#include "glitch/renderer/camera.h"
#include "glitch/renderer/types.h"

#ifdef GL_DEBUG_BUILD
#include "glitch/debug/debug_panel.h"
#endif

Renderer::Renderer() :
		device(Application::get_instance()->get_rendering_device()),
		backend(device->get_backend()) {
	// scene buffer
	scene_data_sbo = StorageBuffer::create(sizeof(SceneData), &scene_data);
	push_constants.scene_buffer = scene_data_sbo->get_device_address();
}

Renderer::~Renderer() { device->wait_for_device(); }

void Renderer::submit(const DrawingContext& p_ctx) {
	GL_PROFILE_SCOPE;

	if (!p_ctx.scene_graph) {
		GL_LOG_WARNING("No SceneGraph assigned to render!");
		return;
	}

	const RenderQueue renderables = _preprocess_render(p_ctx);

	// Apply resolution setting
	device->set_render_scale(p_ctx.settings.resolution_scale);

	CommandBuffer cmd = device->begin_render();
	{
		device->clear_pass(cmd, p_ctx.settings.clear_color);

		_geometry_pass(cmd, renderables, p_ctx.settings);
	}
	device->end_render();

#ifdef GL_DEBUG_BUILD
	device->imgui_begin();
	DebugPanel::draw(p_ctx.scene_graph->get_root());
	device->imgui_end();
#endif
}

void Renderer::submit_func(RenderFunc&& p_func) {
	render_funcs.push_back(p_func);
}

RenderQueue Renderer::_preprocess_render(const DrawingContext& p_ctx) {
	GL_PROFILE_SCOPE;

	// Update scene transforms
	// TODO: dirty flags to cache
	p_ctx.scene_graph->update_transforms();

	// Set camera and scene data
	camera = p_ctx.camera;
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

	// Construct a frustum culled render queue to render only visible primitives
	Frustum view_frustum = Frustum::from_view_proj(scene_data.view_projection);
	RenderQueue render_queue =
			p_ctx.scene_graph->construct_render_queue(view_frustum);

	// Multilevel sorting by used pipeline and z index
	render_queue.sort();

	return render_queue;
}

void Renderer::_geometry_pass(CommandBuffer p_cmd,
		const RenderQueue& p_render_queue, const RendererSettings& p_settings) {
	if (p_render_queue.empty()) {
		return;
	}

	device->begin_rendering(p_cmd, p_settings);

	// TODO: this should probably has their own render pass and a priority
	// parameter should be given
	for (auto& render_func : render_funcs) {
		render_func(p_cmd);
	}
	render_funcs.clear();

	// TODO: preprocess and do an instanced rendering and ibl

	Pipeline bound_pipeline = GL_NULL_HANDLE;
	for (const RenderObject& renderable : p_render_queue) {
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
				p_cmd, primitive->index_buffer, 0, INDEX_TYPE_UINT32);

		backend->command_draw_indexed(p_cmd, primitive->index_count);

		{
			ApplicationPerfStats& stats =
					Application::get_instance()->get_perf_stats();

			stats.renderer_stats.draw_calls++;
			stats.renderer_stats.index_count += primitive->index_count;
		}
	}

	device->end_rendering(p_cmd);
}
