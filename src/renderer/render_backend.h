#pragma once

#include "core/window.h"

#define GL_DEFINE_HANDLE(object) typedef struct object##_T* object;

GL_DEFINE_HANDLE(Context)

GL_DEFINE_HANDLE(Buffer)
GL_DEFINE_HANDLE(Texture)
GL_DEFINE_HANDLE(CommandPool)
GL_DEFINE_HANDLE(CommandBuffer)
GL_DEFINE_HANDLE(Swapchain)
GL_DEFINE_HANDLE(Pipeline)
GL_DEFINE_HANDLE(Shader)
GL_DEFINE_HANDLE(UniformSet)
GL_DEFINE_HANDLE(Fence)
GL_DEFINE_HANDLE(Semaphore)

class RenderBackend {
public:
	virtual Context init(Ref<Window> p_window) = 0;
	virtual void shutdown(Context p_context) = 0;
};
