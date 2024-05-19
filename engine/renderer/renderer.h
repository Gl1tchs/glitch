#pragma once

#include "core/window.h"
#include "renderer/mesh.h"

enum class RenderBackend {
	Vulkan,
	None,
};

[[nodiscard]] RenderBackend find_proper_backend() noexcept;

class Renderer {
public:
	virtual ~Renderer() = default;

	virtual void submit_mesh(Ref<Mesh> mesh) = 0;

	// record drawing commands and submit them
	virtual void draw() = 0;

	static Ref<Renderer> create(RenderBackend backend, Ref<Window> window);

	static RenderBackend get_backend();
};
