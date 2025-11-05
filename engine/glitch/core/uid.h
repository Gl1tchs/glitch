/**
 * @file uid.h
 *
 */

#pragma once

namespace gl {

// TODO: we made this 32-bits for lua compability but if you ever want
// more concurrent entities (4.3B is the limit now) make this 64-bit again

/**
 * 32-bit randomized unique identifier.
 */
struct GL_API UID {
	uint32_t value;

	UID();
	UID(const uint32_t& p_uuid);
	UID(uint32_t&& p_uuid);
	UID(const UID&) = default;

	UID& operator=(const UID& p_other);
	UID& operator=(UID&& p_other);

	UID& operator=(const uint32_t& p_other);
	UID& operator=(uint32_t&& p_other);

	bool is_valid() const { return value != 0; }

	operator uint32_t() const { return value; }
};

inline const UID INVALID_UID = 0;

} //namespace gl

namespace std {
template <typename T> struct hash;

template <> struct hash<gl::UID> {
	size_t operator()(const gl::UID& p_uuid) const { return (uint32_t)p_uuid; }
};
} //namespace std
