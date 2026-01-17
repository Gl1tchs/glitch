#if !defined(GL_PLATFORM_LINUX)
#error "Unix platform specific code can not run on this system."
#else

#include "glitch/platform/os.h"

#include <cstdlib>
#include <cstring>

namespace gl {
namespace os {

const char* getenv(const char* var) { return ::getenv(var); }

bool setenv(const char* name, const char* value) {
	if (!value || strcmp(value, "") == 0) {
		return ::unsetenv(name) == 0;
	}

	return ::setenv(name, value, 1) == 0;
}

} //namespace os
} //namespace gl

#endif
