/**
 * @file mesh_pass.h
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

	struct alignas(16) SceneBuffer {
		glm::mat4 view_projection;

		glm::vec4 camera_position;

		int num_point_lights;

		float _pad0[3];

		DirectionalLight directional_light;
		std::array<PointLight, 16> point_lights;
	};

	struct alignas(16) PushConstants {
		glm::mat4 transform;
		BufferDeviceAddress vertex_buffer;
		BufferDeviceAddress scene_buffer;
	};

	virtual ~MeshPass() = default;

	void setup(Renderer& p_renderer) override;
	void execute(CommandBuffer p_cmd, Renderer& p_renderer) override;

	void set_camera(const PerspectiveCamera& p_camera);
	void set_scene_graph(SceneGraph* p_graph);

private:
	SceneGraph* graph;

	PerspectiveCamera camera;

	PushConstants push_constants = {};
	SceneBuffer scene_data;
	size_t scene_data_hash;
	Ref<StorageBuffer> scene_data_sbo;
};

} //namespace gl