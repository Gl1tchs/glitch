#pragma once

#include "gl/core/window.h"
#include "gl/renderer/camera.h"

#include "gl/renderer/material.h"
#include "gl/renderer/mesh.h"

enum class RenderBackend {
	Vulkan,
	None,
};

[[nodiscard]] RenderBackend find_proper_backend() noexcept;

class Renderer {
public:
	virtual ~Renderer() = default;

	virtual void submit_mesh(
			Ref<Mesh> mesh, Ref<MaterialInstance> material) = 0;

	virtual void attach_camera(Camera* camera) = 0;

	// record drawing commands and submit them
	virtual void draw() = 0;

	// wait for all operations to finish
	virtual void wait_for_device() = 0;

	static Ref<Renderer> create(RenderBackend backend, Ref<Window> window);

	static RenderBackend get_backend();
};
