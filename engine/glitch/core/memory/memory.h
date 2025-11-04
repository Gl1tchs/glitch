#pragma once

#define GL_ALLOC(m_type) (m_type*)malloc(sizeof(m_type))

#define GL_ALLOC_ARRAY(m_type, m_size) (m_type*)malloc(m_size * sizeof(m_type))

#define GL_FREE(m_ptr) free(m_ptr)

namespace gl {

inline constexpr size_t align_up(size_t p_offset, size_t p_alignment) {
	return (p_offset + p_alignment - 1) & ~(p_alignment - 1);
}

} //namespace gl
