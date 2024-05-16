#pragma once

#include "core/window.h"

enum class RenderBackend {
	Vulkan,
	None,
};

[[nodiscard]] RenderBackend find_proper_backend() noexcept;

class Renderer {
public:
	virtual ~Renderer() = default;

	// record drawing commands and submit them
	virtual void draw() = 0;

	static Ref<Renderer> create(RenderBackend backend, Ref<Window> window);
};
