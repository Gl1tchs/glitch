#pragma once

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

#ifdef GL_EXPORT
#ifdef _MSC_VER
#define GL_API __declspec(dllexport)
#else
#define GL_API __attribute__((visibility("default")))
#endif // _MSC_VER
#else
#ifdef _MSC_VER
#define GL_API __declspec(dllimport)
#else
#define GL_API
#endif // _MSC_VER
#endif // GL_EXPORT

#define GL_BIND_FUNC(fn)                                                       \
	[this](auto&&... args) -> decltype(auto) {                                 \
		return this->fn(std::forward<decltype(args)>(args)...);                \
	}

template <typename T> using Optional = std::optional<T>;
