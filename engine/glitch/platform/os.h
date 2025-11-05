#pragma once

namespace gl {

namespace os {

GL_API const char* getenv(const char* p_name);

GL_API bool setenv(const char* p_name, const char* p_value);

} //namespace os

} //namespace gl