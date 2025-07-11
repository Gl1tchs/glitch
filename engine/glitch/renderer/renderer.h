/**
 * @file scene_renderer.h
 */

#pragma once

#include "glitch/renderer/camera.h"
#include "glitch/renderer/drawing_context.h"
#include "glitch/renderer/render_device.h"
#include "glitch/renderer/storage_buffer.h"

struct SceneData {
	glm::mat4 view_projection;
	glm::vec3 camera_position;
};

struct PushConstants {
	BufferDeviceAddress vertex_buffer;
	BufferDeviceAddress scene_buffer;
	glm::mat4 transform;
};

struct RendererSettings {
	Color clear_color = Color(0.1f, 0.1f, 0.1f, 1.0f);
};

/**
 * High level rendering interface
 */
class GL_API Renderer {
public:
	using RenderFunc = std::function<void(CommandBuffer)>;

	Renderer();
	~Renderer();

	void submit(const DrawingContext& p_ctx);

	/**
	 * Push a rendering function into stack using a render pass
	 */
	void submit_func(RenderFunc&& p_func);

	void set_clear_color(const Color& p_color);

private:
	RenderQueue _preprocess_render(const DrawingContext& p_ctx);

	void _geometry_pass(CommandBuffer p_cmd, const RenderQueue& p_render_queue);

private:
	Ref<RenderDevice> device;
	Ref<RenderBackend> backend;

	PushConstants push_constants = {};

	PerspectiveCamera camera;
	Transform camera_transform;

	SceneData scene_data;
	size_t scene_data_hash;
	Ref<StorageBuffer> scene_data_sbo;

	std::vector<RenderFunc> render_funcs;

	RendererSettings settings = {};
};
