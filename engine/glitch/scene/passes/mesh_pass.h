/**
 * @file mesh_pass.h
 *
 */

#pragma once

#include "glitch/renderer/camera.h"
#include "glitch/renderer/graphics_pass.h"
#include "glitch/renderer/light_sources.h"
#include "glitch/renderer/material.h"
#include "glitch/renderer/storage_buffer.h"
#include "glitch/renderer/texture.h"
#include "glitch/scene/scene.h"

namespace gl {

class MeshPass : public GraphicsPass {
public:
	GL_DEFINE_GRAPHICS_PASS("Mesh Pass")

	struct alignas(16) SceneBuffer {
		Mat4 view_projection;

		Vec4f camera_position;

		int num_point_lights;

		float _pad0[3];

		DirectionalLight directional_light;
		std::array<PointLight, 16> point_lights;
	};

	struct alignas(16) PushConstants {
		Mat4 transform;
		BufferDeviceAddress vertex_buffer;
		BufferDeviceAddress scene_buffer;
	};

	virtual ~MeshPass();

	void setup(Renderer& renderer) override;
	void execute(CommandBuffer cmd, Renderer& renderer) override;

	void set_scene(std::shared_ptr<Scene> scene);

private:
	enum class ScenePreprocessError {
		NONE,
		NO_CAMERA,
	};

	ScenePreprocessError _preprocess_scene();

private:
	std::shared_ptr<Scene> _scene;

	std::optional<PerspectiveCamera> _camera;

	PushConstants _push_constants = {};
	SceneBuffer _scene_data;
	size_t _scene_data_hash;
	std::shared_ptr<StorageBuffer> _scene_data_sbo;

	std::shared_ptr<Texture> _default_texture = nullptr;
	std::shared_ptr<Material> _default_material = nullptr;
};

size_t hash64(const MeshPass::SceneBuffer& buf);

} //namespace gl