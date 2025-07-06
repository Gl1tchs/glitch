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
	scene_data_buffer = backend->buffer_create(sizeof(SceneData),
			BUFFER_USAGE_STORAGE_BUFFER_BIT |
					BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
					BUFFER_USAGE_TRANSFER_SRC_BIT,
			MEMORY_ALLOCATION_TYPE_CPU);

	{
		SceneData* scene_data_ptr =
				(SceneData*)backend->buffer_map(scene_data_buffer);
		memcpy(scene_data_ptr, &scene_data, sizeof(SceneData));
		backend->buffer_unmap(scene_data_buffer);
	}

	push_constants.scene_buffer =
			backend->buffer_get_device_address(scene_data_buffer);
}

Renderer::~Renderer() {
	device->wait_for_device();

	backend->buffer_free(scene_data_buffer);
}

void Renderer::submit(const DrawingContext& p_ctx) {
	GL_PROFILE_SCOPE;

	if (!p_ctx.scene_graph) {
		GL_LOG_WARNING("No SceneGraph assigned to render!");
		return;
	}

	_preprocess_render(p_ctx);

	CommandBuffer cmd = device->begin_render();
	{
		_geometry_pass(cmd, p_ctx);
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

void Renderer::_preprocess_render(const DrawingContext& p_ctx) {
	GL_PROFILE_SCOPE;

	// Update scene transforms
	// TODO: dirty flags to cache
	p_ctx.scene_graph->update_transforms();

	// TODO:
	// 1. Frustum culling (SceneNode → visible_nodes)
	// 2. Material sorting:
	//    - Opaque first (sort front-to-back)
	//    - Then alpha-blended (sort back-to-front)
	// 3. Batching: group meshes with same pipeline+material

	// Set camera and scene data
	camera = p_ctx.camera;
	camera_transform = p_ctx.camera_transform;

	camera.aspect_ratio =
			Application::get_instance()->get_window()->get_aspect_ratio();

	scene_data.view_projection = camera.get_projection_matrix() *
			camera.get_view_matrix(camera_transform);
	scene_data.camera_position = camera_transform.position;

	// Reupload the scene buffer if it's updated
	size_t hash = hash64(scene_data.view_projection);
	hash_combine(hash, hash64(scene_data.camera_position));

	// TODO: this is very inefficient
	if (scene_data_hash != hash) {
		SceneData* scene_data_ptr =
				(SceneData*)backend->buffer_map(scene_data_buffer);
		memcpy(scene_data_ptr, &scene_data, sizeof(SceneData));
		backend->buffer_unmap(scene_data_buffer);

		scene_data_hash = hash;
	}

	// TODO: Sort materials
}

void Renderer::_geometry_pass(
		CommandBuffer p_cmd, const DrawingContext& p_ctx) {
	backend->command_begin_render_pass(p_cmd, device->get_render_pass(),
			device->get_current_frame_buffer(), device->get_draw_extent(),
			settings.clear_color);

	// TODO: this should probably has their own render pass and a priority
	// parameter should be given
	for (auto& render_func : render_funcs) {
		render_func(p_cmd);
	}
	render_funcs.clear();

	// Render nodes individually
	// TODO: preprocess and do an instanced rendering and ibl
	_traverse_node_render(p_cmd, p_ctx.scene_graph->get_root());

	backend->command_end_render_pass(p_cmd);
}

void Renderer::_traverse_node_render(
		CommandBuffer p_cmd, const Ref<SceneNode>& p_node) {
	if (p_node->mesh) {
		_render_mesh(p_cmd, p_node->world_transform, p_node->mesh);
	}

	// Render child nodes as well
	for (Ref<SceneNode> child : p_node->children) {
		_traverse_node_render(p_cmd, child);
	}
}

void Renderer::_render_mesh(CommandBuffer p_cmd, const glm::mat4& p_transform,
		const Ref<Mesh>& p_mesh) {
	for (const auto& primitive : p_mesh->primitives) {
		// Of course we need to bind the pipeline if it is really necessary
		// but currently we don't have a mesh system. This same principle
		// can also be told for drawing cubes only so i will just mark it as
		// TODO:

		backend->command_bind_graphics_pipeline(
				p_cmd, primitive->material->definition->pipeline);

		// set = 0 material data
		backend->command_bind_uniform_sets(p_cmd,
				primitive->material->definition->shader, 0,
				primitive->material->uniform_set);

		// Push constants
		{
			push_constants.vertex_buffer = primitive->vertex_buffer_address;

			// Object transformation
			push_constants.transform = p_transform;

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
}

void Renderer::set_clear_color(const Color& p_color) {
	settings.clear_color = p_color;
}
