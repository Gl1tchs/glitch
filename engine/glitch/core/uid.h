/**
 * @file uid.h
 *
 */

#pragma once

#include "glitch/core/defines.h"
#include "glitch/core/json.h"

#include <cstdint>

namespace gl {

// TODO: we made this 32-bits for lua compability but if you ever want
// more concurrent entities (4.3B is the limit now) make this 64-bit again

/**
 * 32-bit randomized unique identifier.
 */
struct GL_API UID {
	uint32_t value;

	UID();
	UID(const uint32_t& uuid);
	UID(uint32_t&& uuid);
	UID(const UID&) = default;

	UID& operator=(const UID& other);
	UID& operator=(UID&& other);

	UID& operator=(const uint32_t& other);
	UID& operator=(uint32_t&& other);

	bool is_valid() const { return value != 0; }

	bool operator==(const UID& other) const { return value == other.value; }
	bool operator!=(const UID& other) const { return value != other.value; }

	operator uint32_t() const { return value; }
};

inline const UID INVALID_UID = 0;

void to_json(json& j, const UID& uid);
void from_json(const json& j, UID& uid);

} //namespace gl

namespace std {
template <typename T> struct hash;

template <> struct hash<gl::UID> {
	size_t operator()(const gl::UID& uuid) const { return (uint32_t)uuid; }
};
} //namespace std
