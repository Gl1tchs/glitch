#pragma once

#include "core/window.h"

#include "renderer/types.h"

class RenderBackend {
public:
	virtual ~RenderBackend() = default;

	virtual Context init(Ref<Window> p_window) = 0;
	virtual void shutdown(Context p_context) = 0;

	virtual void wait_for_device() = 0;

	virtual CommandQueue get_command_queue(QueueType p_type) = 0;
};
