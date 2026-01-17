#pragma once

#include "glitch/core/defines.h"

namespace gl {

namespace os {

GL_API const char* getenv(const char* name);

/**
 * Sets environment variable if `name` or `value` is not null,
 * if `name` is not null and `value` is null then the variable
 * is going to be unsetted.
 */
GL_API bool setenv(const char* name, const char* value);

} //namespace os

} //namespace gl
