#pragma once

#include "core/window.h"

enum class RenderBackend {
	Vulkan,
	None,
};

[[nodiscard]] RenderBackend find_proper_backend() noexcept;

class IRenderer {
public:
	virtual ~IRenderer() = default;

	static Ref<IRenderer> create(RenderBackend backend, Ref<Window> window);
};
