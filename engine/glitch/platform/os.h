#pragma once

namespace gl {

namespace os {

GL_API const char* getenv(const char* p_name);

/**
 * Sets environment variable if `p_name` or `p_value` is not null,
 * if `p_name` is not null and `p_value` is null then the variable
 * is going to be unsetted.
 */
GL_API bool setenv(const char* p_name, const char* p_value);

} //namespace os

} //namespace gl
