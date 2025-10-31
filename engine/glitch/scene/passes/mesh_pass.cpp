#include "glitch/scene/passes/mesh_pass.h"

#include "glitch/core/application.h"
#include "glitch/scene/components.h"
#include "glitch/scene/entity.h"

namespace gl {

void MeshPass::setup(Renderer& p_renderer) {
	GL_PROFILE_SCOPE;

	scene_data_sbo = StorageBuffer::create(sizeof(SceneBuffer), &scene_data);
	push_constants.scene_buffer = scene_data_sbo->get_device_address();
}

void MeshPass::execute(CommandBuffer p_cmd, Renderer& p_renderer) {
	GL_PROFILE_SCOPE;

	if (!scene) {
		GL_LOG_ERROR("[MeshPass::execute] No scene graph bound to render");
		return;
	}

	// Set camera, light sources and do frustum culling
	ScenePreprocessError err = _preprocess_scene();

	if (err != ScenePreprocessError::NONE) {
		// If there was more error codes a switch statements would be a better
		// idea but for now this has less instructions and no compiler warning
		if (err == ScenePreprocessError::NO_CAMERA) {
			GL_LOG_ERROR("[MeshPass::execute] No camera found in the registry "
						 "to render!");
		}
		return;
	}

	Ref<RenderBackend> backend = p_renderer.get_backend();

	p_renderer.begin_rendering(p_cmd,
			p_renderer.get_render_image("geo_albedo").value(),
			p_renderer.get_render_image("geo_depth").value());

	Pipeline bound_pipeline = GL_NULL_HANDLE;
	for (Entity entity : scene->view<MeshComponent>()) {
		const MeshComponent* mc = entity.get_component<MeshComponent>();
		if (!mc->visible) {
			// probably culled or mesh doesn't exist
			continue;
		}

		Ref<Mesh> mesh = MeshSystem::get_mesh(mc->mesh);
		if (!mesh) {
			continue;
		}

		for (Ref<MeshPrimitive> primitive : mesh->primitives) {
			// Bind the pipeline if not already bound
			Pipeline pipeline = primitive->material->get_pipeline();
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
				push_constants.transform = entity.get_transform().to_mat4();

				backend->command_push_constants(p_cmd,
						primitive->material->get_shader(), 0,
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
	}

	p_renderer.end_rendering(p_cmd);
}

void MeshPass::set_scene(Ref<Scene> p_scene) { scene = p_scene; }

MeshPass::ScenePreprocessError MeshPass::_preprocess_scene() {
	Optional<Transform> camera_transform = std::nullopt;
	for (Entity entity : scene->view<CameraComponent>()) {
		CameraComponent* cc = entity.get_component<CameraComponent>();
		if (cc->enabled) {
			camera_transform = entity.get_transform();
			camera = cc->camera;
			// TODO: more sophisticated solution
			break;
		}
	}

	if (!camera) {
		return ScenePreprocessError::NO_CAMERA;
	}

	camera.value().aspect_ratio =
			Application::get_instance()->get_window()->get_aspect_ratio();

	Optional<DirectionalLight> directional_light;
	for (Entity entity : scene->view<DirectionalLight>()) {
		const DirectionalLight* dl = entity.get_component<DirectionalLight>();
		directional_light = *dl;
	}

	std::vector<PointLight> point_lights;
	for (Entity entity : scene->view<PointLight>()) {
		PointLight* pl = entity.get_component<PointLight>();
		pl->position = glm::vec4(entity.get_transform().get_position(), 0.0f);

		point_lights.push_back(*pl);
	}

	// Upload scene data to the GPU
	{
		scene_data.view_projection = camera.value().get_projection_matrix() *
				camera.value().get_view_matrix(*camera_transform);
		scene_data.camera_position =
				glm::vec4(camera_transform.value().get_position(), 0.0f);

		// Directional light
		scene_data.directional_light =
				directional_light ? *directional_light : DirectionalLight{};

		// Copy point lights
		const size_t count = std::min<size_t>(16, point_lights.size());
		std::copy_n(
				point_lights.begin(), count, scene_data.point_lights.begin());
		scene_data.num_point_lights = count;

		// Reupload the scene buffer if it's updated
		const size_t hash = hash64(scene_data);
		if (scene_data_hash != hash) {
			scene_data_sbo->upload(&scene_data);
			scene_data_hash = hash;
		}
	}

	// Construct a frustum culled render queue to render only visible primitives
	Frustum view_frustum = Frustum::from_view_proj(scene_data.view_projection);

	// Basic frustum culling and material updating
	for (Entity entity : scene->view<MeshComponent>()) {
		MeshComponent* mc = entity.get_component<MeshComponent>();

		Ref<Mesh> mesh = MeshSystem::get_mesh(mc->mesh);
		if (!mesh) {
			mc->visible = false;
			continue;
		}

		for (const auto& prim : mesh->primitives) {
			// If objects is not inside of the view frustum, discard it.
			const AABB aabb =
					prim->aabb.transform(entity.get_transform().to_mat4());
			if (!aabb.is_inside_frustum(view_frustum)) {
				mc->visible = false;
				continue;
			}

			// TODO: if material component then set

			if (prim->material->is_dirty()) {
				prim->material->upload();
			}

			mc->visible = true;
		}
	}

	return ScenePreprocessError::NONE;
}

size_t hash64(const MeshPass::SceneBuffer& p_buf) {
	size_t hash = hash64(p_buf.view_projection);
	hash_combine(hash, hash64(p_buf.camera_position));

	hash_combine(hash, hash64(p_buf.directional_light.direction));
	hash_combine(hash, hash64(p_buf.directional_light.color));

	hash_combine(hash, hash64(p_buf.num_point_lights));
	for (int i = 0; i < p_buf.num_point_lights; i++) {
		PointLight pl = p_buf.point_lights[i];
		hash_combine(hash, hash64(pl.position));
		hash_combine(hash, hash64(pl.color));
		hash_combine(hash, hash64(pl.linear));
		hash_combine(hash, hash64(pl.quadratic));
	}

	return hash;
}

} //namespace gl
