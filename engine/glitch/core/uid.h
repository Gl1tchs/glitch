#pragma once

namespace gl {

/**
 * 64-bit randomized unique identifier.
 */
struct GL_API UID {
	uint64_t value;

	UID();
	UID(const uint64_t& p_uuid);
	UID(uint64_t&& p_uuid);
	UID(const UID&) = default;

	UID& operator=(const UID& p_other);
	UID& operator=(UID&& p_other);

	UID& operator=(const uint64_t& p_other);
	UID& operator=(uint64_t&& p_other);

	bool is_valid() const { return value != 0; }

	operator uint64_t() const { return value; }
};

inline const UID INVALID_UID = 0;

} //namespace gl

namespace std {
template <typename T> struct hash;

template <> struct hash<gl::UID> {
	size_t operator()(const gl::UID& p_uuid) const { return (uint64_t)p_uuid; }
};
} //namespace std
