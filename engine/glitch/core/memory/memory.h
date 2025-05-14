#pragma once

#include <memory>

#define GL_ALLOC(m_type) (m_type*)malloc(sizeof(m_type))

#define GL_ALLOC_ARRAY(m_type, m_size) (m_type*)malloc(m_size * sizeof(m_type))

#define GL_FREE(m_ptr) free(m_ptr)

template <typename T> using Weak = std::weak_ptr<T>;

template <typename T> using Scope = std::unique_ptr<T>;

template <typename T, typename... Args>
inline constexpr Scope<T> create_scope(Args&&... args) {
	return std::make_unique<T>(std::forward<Args>(args)...);
}

template <typename T> using Ref = std::shared_ptr<T>;

template <typename T, typename... Args>
inline constexpr Ref<T> create_ref(Args&&... args) {
	return std::make_shared<T>(std::forward<Args>(args)...);
}
