#if !defined(GL_PLATFORM_LINUX)
#error "Unix platform specific code can not run on this system."
#else

#include "glitch/platform/os.h"

namespace gl {
namespace os {

const char* getenv(const char* p_var) { return ::getenv(p_var); }

bool setenv(const char* p_name, const char* p_value) {
	return ::putenv(std::format("{}={}", p_name, p_value).c_str());
}

} //namespace os
} //namespace gl

#endif