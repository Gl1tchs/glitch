#include "glitch/renderer/scene_renderer.h"

#include "glitch/core/application.h"
#include "glitch/core/color.h"
#include "glitch/core/transform.h"
#include "glitch/renderer/camera.h"
#include "glitch/renderer/material.h"
#include "glitch/renderer/render_backend.h"
#include "glitch/renderer/types.h"
#include "glitch/scene/components.h"

struct MeshVertex {
	glm::vec3 position;
	float uv_x;
	glm::vec3 normal;
	float uv_y;
};

struct PushConstants {
	BufferDeviceAddress vertex_buffer;
};

Buffer vertex_buffer = GL_NULL_HANDLE;
Buffer index_buffer = GL_NULL_HANDLE;

Buffer scene_data_buffer = GL_NULL_HANDLE;
UniformSet scene_data = GL_NULL_HANDLE;

PushConstants push_constants = {};

SceneRenderer::SceneRenderer() :
		renderer(Application::get_instance()->get_renderer()),
		backend(renderer->get_backend()) {
	// create the default 1x1 white texture.
	default_texture = Texture::create(COLOR_WHITE, { 1, 1 });

	const std::vector<MeshVertex> vertices = {
		{ { -1.0f, -1.0f, 1.0f } }, //0
		{ { 1.0f, -1.0f, 1.0f } }, //1
		{ { -1.0f, 1.0f, 1.0f } }, //2
		{ { 1.0f, 1.0f, 1.0f } }, //3
		{ { -1.0f, -1.0f, -1.0f } }, //4
		{ { 1.0f, -1.0f, -1.0f } }, //5
		{ { -1.0f, 1.0f, -1.0f } }, //6
		{ { 1.0f, 1.0f, -1.0f } }, //7
	};

	vertex_buffer = backend->buffer_create(vertices.size() * sizeof(MeshVertex),
			BUFFER_USAGE_STORAGE_BUFFER_BIT |
					BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
					BUFFER_USAGE_TRANSFER_DST_BIT,
			MEMORY_ALLOCATION_TYPE_CPU);

	void* mapped_data = backend->buffer_map(vertex_buffer);
	memcpy(mapped_data, vertices.data(), vertices.size() * sizeof(MeshVertex));
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

	unlit_material = create_ref<MaterialUnlit>();

	// uniform set 0
	scene_data_buffer = backend->buffer_create(sizeof(SceneData),
			BUFFER_USAGE_UNIFORM_BUFFER_BIT | BUFFER_USAGE_TRANSFER_SRC_BIT,
			MEMORY_ALLOCATION_TYPE_CPU);
	SceneData* scene_data_ptr =
			(SceneData*)backend->buffer_map(scene_data_buffer);
	PerspectiveCamera cam;
	Transform cam_transform = { .local_position = { 0, 0, 4 } };

	scene_data_ptr->camera_position = cam_transform.get_position();
	cam.aspect_ratio =
			Application::get_instance()->get_window()->get_aspect_ratio();

	scene_data_ptr->view_projection =
			cam.get_projection_matrix() * cam.get_view_matrix(cam_transform);
	backend->buffer_unmap(scene_data_buffer);

	ShaderUniform scene_data_uniform;
	scene_data_uniform.type = UNIFORM_TYPE_UNIFORM_BUFFER;
	scene_data_uniform.binding = 0;
	scene_data_uniform.data.push_back(scene_data_buffer);

	scene_data = backend->uniform_set_create(
			scene_data_uniform, unlit_material->shader, 0);

	push_constants.vertex_buffer =
			backend->buffer_get_device_address(vertex_buffer);
}

SceneRenderer::~SceneRenderer() {
	renderer->wait_for_device();

	backend->buffer_free(vertex_buffer);
	backend->buffer_free(index_buffer);
	backend->uniform_set_free(scene_data);
	backend->buffer_free(scene_data_buffer);

	_cleanup_scene_resources();
}

void SceneRenderer::render_scene(Scene* p_scene) {
	scene = p_scene;

	// prepare entities and components to draw
	_prepare_scene();

	CommandBuffer cmd = renderer->begin_render();

	backend->command_begin_rendering(cmd, renderer->get_draw_extent(),
			renderer->get_draw_image(), renderer->get_depth_image());

	for (auto entity : scene->view<MaterialComponent>()) {
		const MaterialComponent* material_comp =
				scene->get<MaterialComponent>(entity);

		const Ref<MaterialInstance> material = material_comp->instance;

		// Of course we need to bind the pipeline if it is really necessary
		// but currently we don't have a mesh system. This same principle can
		// also be told for drawing cubes only so i will just mark it as
		// TODO:

		backend->command_bind_graphics_pipeline(cmd, material->pipeline);

		// Descriptors
		std::vector<UniformSet> uniform_sets = {
			scene_data,
			material->uniform_set,
		};

		// set = 0 scene data
		// set = 1 material data
		backend->command_bind_uniform_sets(
				cmd, material->shader, 0, uniform_sets);

		// Push constants
		backend->command_push_constants(cmd, material->shader, 0,
				sizeof(PushConstants), &push_constants);

		backend->command_bind_index_buffer(
				cmd, index_buffer, 0, INDEX_TYPE_UINT32);

		backend->command_draw_indexed(cmd, 36);
	}

	backend->command_end_rendering(cmd);

	renderer->end_render();
}

void SceneRenderer::_prepare_scene() {
	for (auto entity : scene->view<MaterialComponent>()) {
		MaterialComponent* material_comp =
				scene->get<MaterialComponent>(entity);

		const size_t hash = hash64(*material_comp);

		// update the instance if it doesn't exist or updated
		if (material_comp->instance != nullptr && hash == material_comp->hash) {
			continue;
		}

		// TODO: buffer creation should be cached
		Buffer material_data = backend->buffer_create(
				sizeof(MaterialParameters),
				BUFFER_USAGE_UNIFORM_BUFFER_BIT | BUFFER_USAGE_TRANSFER_SRC_BIT,
				MEMORY_ALLOCATION_TYPE_CPU);

		// TODO: for a per-frame buffer it is not essential to do this.
		MaterialParameters* material_data_ptr =
				(MaterialParameters*)backend->buffer_map(material_data);
		{
			material_data_ptr->base_color = material_comp->base_color;
			material_data_ptr->metallic = material_comp->metallic;
			material_data_ptr->roughness = material_comp->roughness;
		}
		backend->buffer_unmap(material_data);

		MaterialResources resources;
		resources.material_data = material_data;
		resources.albedo_texture = material_comp->albedo_texture
				? material_comp->albedo_texture
				: default_texture;
		resources.normal_texture = material_comp->normal_texture
				? material_comp->normal_texture
				: default_texture;

		switch (material_comp->type) {
			case MATERIAL_TYPE_UNLIT: {
				// call the destructor to free resources
				material_comp->instance =
						unlit_material->create_instance(resources);
				break;
			}
		}

		material_comp->hash = hash;
	}
}

void SceneRenderer::_cleanup_scene_resources() {
	for (auto entity : scene->view<MaterialComponent>()) {
		MaterialComponent* material_comp =
				scene->get<MaterialComponent>(entity);
		if (material_comp->instance) {
			material_comp->instance.reset();
		}
	}
}
