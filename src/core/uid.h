#pragma once

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

namespace std {
template <typename T> struct hash;

template <> struct hash<UID> {
	size_t operator()(const UID& p_uuid) const { return (uint64_t)p_uuid; }
};
} //namespace std
