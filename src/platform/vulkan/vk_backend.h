#pragma once

#include "renderer/render_backend.h"

#include "core/deletion_queue.h"

struct VulkanContext;

class VulkanRenderBackend : public RenderBackend {
public:
	virtual ~VulkanRenderBackend() = default;

	Context init(Ref<Window> p_window) override;
	void shutdown(Context p_context) override;

	void wait_for_device() override;

	CommandQueue get_command_queue(QueueType p_type) override;

private:
	static VulkanContext* s_context;

	DeletionQueue deletion_queue;
};
