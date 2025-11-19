#if !defined(GL_PLATFORM_LINUX)
#error "Unix platform specific code can not run on this system."
#else

#include "glitch/platform/os.h"

namespace gl {
namespace os {

const char* getenv(const char* p_var) { return ::getenv(p_var); }

bool setenv(const char* p_name, const char* p_value) {
	if (!p_value || strcmp(p_value, "") == 0) {
		return ::unsetenv(p_name) == 0;
	}

	return ::setenv(p_name, p_value, 1) == 0;
}

} //namespace os
} //namespace gl

#endif
