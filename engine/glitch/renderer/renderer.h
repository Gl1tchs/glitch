/**
 * @file scene_renderer.h
 */

#pragma once

#include "glitch/renderer/camera.h"
#include "glitch/renderer/drawing_context.h"
#include "glitch/renderer/gltf_loader.h"
#include "glitch/renderer/material.h"
#include "glitch/renderer/render_device.h"

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
	glm::mat4 transform;
};

/**
 * High level rendering interface
 */
class GL_API Renderer {
public:
	Renderer();
	~Renderer();

	void submit(const DrawingContext& p_ctx);

private:
	void _preprocess_render(const DrawingContext& p_ctx);

	void _traverse_node_render(
			CommandBuffer p_cmd, const Ref<SceneNode>& p_node);

	void _render_mesh(CommandBuffer p_cmd, const glm::mat4& p_transform,
			const Ref<Mesh>& p_mesh);

private:
	Ref<RenderDevice> device;
	Ref<RenderBackend> backend;

	PushConstants push_constants = {};

	PerspectiveCamera camera;
	Transform camera_transform;

	SceneData scene_data;
	size_t scene_data_hash;
	Buffer scene_data_buffer;

#ifdef GL_DEBUG_BUILD
	DebugPanel debug_panel;
#endif
};
