#pragma once

#include "renderer/types.h"

namespace vk {

Fence fence_create(Context p_context);

void fence_free(Context p_context, Fence p_fence);

void fence_wait(Context p_context, Fence p_fence);

void fence_reset(Context p_context, Fence p_fence);

Semaphore semaphore_create(Context p_context);

void semaphore_free(Context p_context, Semaphore p_semaphore);

} //namespace vk
