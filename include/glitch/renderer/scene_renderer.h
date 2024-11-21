/**
 * @file scene_renderer.h
 */

#pragma once

#include "glitch/renderer/materials/material_unlit.h"
#include "glitch/renderer/renderer.h"
#include "glitch/scene/scene.h"

struct SceneData {
	glm::mat4 view_projection;
	glm::vec3 camera_position;
};

class GL_API SceneRenderer {
public:
	SceneRenderer();
	~SceneRenderer();

	void render_scene(Scene* p_scene);

private:
	void _prepare_scene();

	void _cleanup_scene_resources();

	// void _render_mesh(const Ref<Mesh> p_mesh, const Ref<Material>
	// p_material);
	// void _render_text(const std::string& p_text, const Ref<Font>
	// p_font, float p_size, const Color& p_color);

private:
	Ref<Renderer> renderer;
	Ref<RenderBackend> backend;

	Scene* scene;

	Ref<MaterialUnlit> unlit_material;

	Ref<Texture> default_texture;
};
