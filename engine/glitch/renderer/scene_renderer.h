/**
 * @file scene_renderer.h
 */

#pragma once

#include "glitch/renderer/material.h"
#include "glitch/renderer/mesh_loader.h"
#include "glitch/renderer/renderer.h"
#include "glitch/scene/scene.h"

#ifdef GL_DEBUG_BUILD
#include "glitch/debug/debug_panel.h"
#endif

struct SceneData {
	glm::mat4 view_projection;
	glm::vec3 camera_position;
};

struct PushConstants {
	BufferDeviceAddress vertex_buffer;
	BufferDeviceAddress scene_buffer;
};

class GL_API SceneRenderer {
public:
	SceneRenderer();
	~SceneRenderer();

	void render_scene(Scene* p_scene);

	// TODO: temporary
	Ref<MeshLoader> get_mesh_loader() { return mesh_loader; }

private:
	void _prepare_scene();

	// void _render_mesh(const Ref<Mesh> p_mesh, const Ref<Material>
	// p_material);
	// void _render_text(const std::string& p_text, const Ref<Font>
	// p_font, float p_size, const Color& p_color);

private:
	Ref<Renderer> renderer;
	Ref<RenderBackend> backend;

	PushConstants push_constants = {};

	Scene* scene;

	SceneData scene_data;
	size_t scene_data_hash;
	Buffer scene_data_buffer;

	Ref<MaterialSystem> material_system;
	Ref<MeshLoader> mesh_loader;

	std::unordered_map<Entity, Ref<MaterialInstance>> materials;
	Ref<Texture> default_texture;

#ifdef GL_DEBUG_BUILD
	DebugPanel debug_panel;
#endif
};
