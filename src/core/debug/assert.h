#pragma once

#include "core/debug/log.h"

#if _WIN32
#define DEBUGBREAK() __debugbreak()
#elif __linux__
#include <signal.h>
#define DEBUGBREAK() raise(SIGTRAP)
#else
#error "Platform doesn't support debugbreak yet!"
#endif

#define GL_EXPAND_MACRO(x) x
#define GL_STRINGIFY_MACRO(x) #x

#define GL_INTERNAL_ASSERT_IMPL(check, msg, ...)                               \
	if (!(check)) {                                                            \
		GL_LOG_FATAL(msg, __VA_ARGS__);                                        \
		DEBUGBREAK();                                                          \
	}

#define GL_INTERNAL_ASSERT_WITH_MSG(check, ...)                                \
	GL_INTERNAL_ASSERT_IMPL(check, "Assertion failed: {}", __VA_ARGS__)

#define GL_INTERNAL_ASSERT_NO_MSG(check)                                       \
	GL_INTERNAL_ASSERT_IMPL(check, "Assertion '{}' failed at {}:{}",           \
			GL_STRINGIFY_MACRO(check), __FILE__, __LINE__)

#define GL_INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro

#define GL_INTERNAL_ASSERT_GET_MACRO(...)                                      \
	GL_EXPAND_MACRO(GL_INTERNAL_ASSERT_GET_MACRO_NAME(__VA_ARGS__,             \
			GL_INTERNAL_ASSERT_WITH_MSG, GL_INTERNAL_ASSERT_NO_MSG))

#define GL_ASSERT(...)                                                         \
	GL_EXPAND_MACRO(GL_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(__VA_ARGS__))
