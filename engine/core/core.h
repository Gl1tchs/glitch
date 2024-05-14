#pragma once

#include "core/assert.h"
#include "core/buffer.h"
#include "core/memory.h"
#include "core/vector.h"

#if __linux__
#define GL_PLATFORM_LINUX 1
#elif __FreeBSD__
#define GL_PLATFORM_BSD
#elif _WIN32 || _WIN64 || __CYGWIN__
#define GL_PLATFORM_WINDOWS 1
#elif __APPLE__
#define GL_PLATFORM_APPLE
#elif __ANDROID__
#define GL_PLATFORM_ANDROID 1
#endif

#define BIND_FUNC(fn)                                                          \
	[this](auto&&... args) -> decltype(auto) {                                 \
		return this->fn(std::forward<decltype(args)>(args)...);                \
	}
