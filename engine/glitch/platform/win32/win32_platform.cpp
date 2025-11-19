#if !defined(GL_PLATFORM_WINDOWS)
#error "Unix platform specific code can not run on this system."
#else

#include "glitch/platform/os.h"

#include <windows.h>

namespace gl {
namespace os {

const char* getenv(const char* p_name) {
	// TODO: maybe better way to do this
	const DWORD buffer_size = 65535;
	static char buffer[buffer_size];
	if (GetEnvironmentVariableA(p_name, buffer, buffer_size)) {
		return buffer;
	} else {
		return 0;
	}
}

bool setenv(const char* p_name, const char* p_value) {
	return SetEnvironmentVariable(p_name, p_value);
}

} //namespace os
} //namespace gl

#endif