#pragma once

#include "core/window.h"

#include "renderer/types.h"

class RenderBackend {
public:
	virtual Context init(Ref<Window> p_window) = 0;
	virtual void shutdown(Context p_context) = 0;
};
