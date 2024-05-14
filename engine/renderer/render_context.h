#pragma once

enum class RenderBackend {
	Vulkan,
	None,
};

[[nodiscard]] RenderBackend find_proper_backend() noexcept;

class Window;

struct RenderContext {
	RenderBackend backend;

	void init(Ref<Window> window);

	void destroy();

private:
	void* native_context;
};
