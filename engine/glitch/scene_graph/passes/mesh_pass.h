/**
 * @file imgui_pass.h
 *
 */

#pragma once

#include "glitch/renderer/camera.h"
#include "glitch/renderer/graphics_pass.h"
#include "glitch/renderer/storage_buffer.h"
#include "glitch/scene_graph/scene_graph.h"

namespace gl {

class MeshPass : public GraphicsPass {
public:
	GL_DEFINE_GRAPHICS_PASS("Mesh Pass")

	struct SceneData {
		glm::mat4 view_projection;
		glm::vec3 camera_position;
	};

	struct PushConstants {
		BufferDeviceAddress vertex_buffer;
		BufferDeviceAddress scene_buffer;
		glm::mat4 transform;
	};

	virtual ~MeshPass();

	void setup(Renderer& p_renderer) override;
	void execute(CommandBuffer p_cmd, Renderer& p_renderer) override;

	void set_camera(const PerspectiveCamera& p_camera);
	void set_scene_graph(SceneGraph* p_graph);

private:
	SceneGraph* graph;

	PerspectiveCamera camera;

	PushConstants push_constants = {};
	SceneData scene_data;
	size_t scene_data_hash;
	Ref<StorageBuffer> scene_data_sbo;
};

} //namespace gl