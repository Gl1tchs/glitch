#pragma once

#include "renderer/render_backend.h"

#include "core/deletion_queue.h"

struct VulkanContext;

class VulkanRenderBackend : public RenderBackend {
public:
	Context init(Ref<Window> p_window) override;
	void shutdown(Context p_context) override;

private:
	static VulkanContext* s_context;

	DeletionQueue deletion_queue;
};
