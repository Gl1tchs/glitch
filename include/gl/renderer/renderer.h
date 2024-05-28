#pragma once

#include "gl/core/window.h"
#include "gl/renderer/camera.h"
#include "gl/renderer/compute.h"
#include "gl/renderer/material.h"
#include "gl/renderer/mesh.h"

enum class RenderBackend {
	Vulkan,
	None,
};

[[nodiscard]] RenderBackend find_proper_backend() noexcept;

struct InstanceSubmitData {
	glm::mat4 transform;
};

struct RendererSettings {
	float render_scale = 1.0f;
	// bool msaa;
};

class Renderer {
public:
	virtual ~Renderer() = default;

	virtual void submit_mesh(Ref<Mesh> mesh, Ref<MaterialInstance> material,
			const InstanceSubmitData& data) = 0;

	virtual void submit_compute_effect(Ref<ComputeEffect> effect) = 0;

	virtual void attach_camera(Camera* camera) = 0;

	// record drawing commands and submit them
	virtual void draw() = 0;

	// wait for all operations to finish
	virtual void wait_for_device() = 0;

	RendererSettings& get_settings();

	static RenderBackend get_backend();

	static Ref<Renderer> create(RenderBackend backend, Ref<Window> window);

private:
	RendererSettings settings{};
};
