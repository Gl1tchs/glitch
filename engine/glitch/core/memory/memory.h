#pragma once

#include <cstddef>
#include <cstdlib>

#define GL_ALLOC(p_type) (p_type*)malloc(sizeof(p_type))

#define GL_ALLOC_ARRAY(p_type, p_size) (p_type*)malloc(p_size * sizeof(p_type))

#define GL_FREE(p_ptr) free(p_ptr)

namespace gl {

inline constexpr size_t align_up(size_t offset, size_t alignment) {
	return (offset + alignment - 1) & ~(alignment - 1);
}

} //namespace gl
