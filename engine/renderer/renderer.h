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

	static Ref<Renderer> create(RenderBackend backend, Ref<Window> window);
};
