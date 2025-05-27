#include "glitch/renderer/scene_renderer.h"

#include "glitch/core/application.h"
#include "glitch/core/transform.h"
#include "glitch/renderer/camera.h"
#include "glitch/renderer/materials/material_unlit.h"
#include "glitch/renderer/mesh_loader.h"
#include "glitch/renderer/render_backend.h"
#include "glitch/renderer/types.h"
#include "glitch/scene/components.h"

SceneRenderer::SceneRenderer() :
		renderer(Application::get_instance()->get_renderer()),
		backend(renderer->get_backend()) {
	// create the default 1x1 white texture.
	default_texture = Texture::create(COLOR_WHITE, { 1, 1 });

	material_system = create_ref<MaterialSystem>();
	material_system->register_definition(
			"mat_unlit", get_unlit_material_definition());

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

SceneRenderer::~SceneRenderer() {
	renderer->wait_for_device();

	backend->buffer_free(scene_data_buffer);
}

void SceneRenderer::render_scene(Scene* p_scene) {
	GL_PROFILE_SCOPE;

	scene = p_scene;

	// prepare entities and components to draw
	_prepare_scene();

	CommandBuffer cmd = renderer->begin_render();

	backend->command_begin_rendering(cmd, renderer->get_draw_extent(),
			renderer->get_draw_image(), renderer->get_depth_image());

	for (auto entity : scene->view<MeshComponent, MaterialComponent>()) {
		const auto [material_comp, mesh_comp] =
				scene->get<MaterialComponent, MeshComponent>(entity);

		const Ref<MaterialInstance>& material = materials.at(entity);
		const Ref<Mesh>& mesh = mesh_loader.get_mesh(mesh_comp->mesh);
		if (!material || !mesh) {
			continue;
		}

		// Of course we need to bind the pipeline if it is really necessary
		// but currently we don't have a mesh system. This same principle can
		// also be told for drawing cubes only so i will just mark it as
		// TODO:

		backend->command_bind_graphics_pipeline(
				cmd, material->definition->pipeline);

		// Descriptors
		std::vector<UniformSet> uniform_sets = {
			material->uniform_set,
		};

		// set = 0 material data
		backend->command_bind_uniform_sets(
				cmd, material->definition->shader, 0, uniform_sets);

		// Push constants
		for (const auto& primitive : mesh->primitives) {
			push_constants.vertex_buffer = primitive->vertex_buffer_address;

			// Object transformation
			if (scene->has<Transform>(entity)) {
				push_constants.transform =
						scene->get<Transform>(entity)->get_transform_matrix();
			} else {
				push_constants.transform = glm::mat4(1.0f);
			}

			backend->command_push_constants(cmd, material->definition->shader,
					0, sizeof(PushConstants), &push_constants);

			backend->command_bind_index_buffer(
					cmd, primitive->index_buffer, 0, INDEX_TYPE_UINT32);

			backend->command_draw_indexed(cmd, primitive->index_count);

			{
				ApplicationPerfStats& stats =
						Application::get_instance()->get_perf_stats();

				stats.renderer_stats.draw_calls++;
				stats.renderer_stats.index_count += primitive->index_count;
			}
		}
	}

	backend->command_end_rendering(cmd);

	renderer->end_render();

#ifdef GL_DEBUG_BUILD
	renderer->imgui_begin();

	debug_panel.draw(scene);

	renderer->imgui_end();
#endif
}

void SceneRenderer::_prepare_scene() {
	GL_PROFILE_SCOPE;

	// Camera and scene data
	for (auto entity : scene->view<Transform, CameraComponent>()) {
		auto [transform, cc] = scene->get<Transform, CameraComponent>(entity);
		if (!cc->enabled) {
			continue;
		}

		cc->camera.aspect_ratio =
				Application::get_instance()->get_window()->get_aspect_ratio();

		scene_data.view_projection = cc->camera.get_projection_matrix() *
				cc->camera.get_view_matrix(*transform);
		scene_data.camera_position = transform->get_position();

		// Update the buffer if changed
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

		break;
	}

	// Meshes
	for (auto entity : scene->view<MeshComponent>()) {
		const MeshComponent* mesh_comp = scene->get<MeshComponent>(entity);
		const Ref<Mesh>& mesh = mesh_loader.get_mesh(mesh_comp->mesh);

		// If mesh doesn't have and external material component create and
		// populate one
		if (!scene->has<MaterialComponent>(entity)) {
			const MeshMaterialParameters& mat_params =
					mesh->primitives.front()->material;

			MaterialComponent* mat = scene->assign<MaterialComponent>(entity);
			mat->base_color = mat_params.base_color;
			mat->metallic = mat_params.metallic;
			mat->roughness = mat_params.roughness;
			mat->albedo_texture = mat_params.albedo_texture;
		}
	}

	// Materials
	for (auto entity : scene->view<MaterialComponent>()) {
		MaterialComponent* material_comp =
				scene->get<MaterialComponent>(entity);

		const size_t hash = hash64(*material_comp);

		// update the instance if it doesn't exist or updated
		if (materials.find(entity) != materials.end() &&
				hash == material_comp->hash) {
			continue;
		}

		switch (material_comp->type) {
			case MATERIAL_TYPE_UNLIT: {
				Ref<MaterialInstance> instance =
						material_system->create_instance("mat_unlit");
				instance->set_param("base_color", material_comp->base_color);
				instance->set_param("metallic", material_comp->metallic);
				instance->set_param("roughness", material_comp->roughness);
				instance->set_param("u_albedo_texture",
						material_comp->albedo_texture
								? material_comp->albedo_texture
								: default_texture);
				instance->upload();

				materials[entity] = instance;
				break;
			}
		}

		material_comp->hash = hash;
	}

	// TODO: Sort materials
}
