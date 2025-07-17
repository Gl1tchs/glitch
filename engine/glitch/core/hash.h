/**
 * @file hash.h
 */

#pragma once

namespace gl {

template <typename T>
inline void hash_combine(std::size_t& p_seed, T const& p_value) {
	p_seed ^= std::hash<T>()(p_value) + 0x9e3779b9 + (p_seed << 6) +
			(p_seed >> 2);
}

template <typename T> inline size_t hash64(const T& p_value) {
	size_t seed = 0;
	hash_combine(seed, p_value);
	return seed;
}

inline size_t hash64(const void* data, size_t size) {
	const uint8_t* bytes = static_cast<const uint8_t*>(data);
	size_t seed = 0;

	for (size_t i = 0; i < size; ++i) {
		hash_combine(seed, bytes[i]);
	}

	return seed;
}

template <typename T> inline size_t hash64(const std::vector<T>& p_value) {
	size_t seed = 0;
	for (size_t i = 0; i < p_value.size(); i++) {
		hash_combine(seed, p_value[i]);
	}
	return seed;
}

template <> inline size_t hash64(const glm::vec3& p_v) {
	size_t seed = 0;
	hash_combine(seed, p_v.x);
	hash_combine(seed, p_v.y);
	hash_combine(seed, p_v.z);
	return seed;
}

template <> inline size_t hash64(const glm::mat4& p_m) {
	size_t seed = 0;
	for (int col = 0; col < 4; ++col) {
		for (int row = 0; row < 4; ++row) {
			hash_combine(seed, p_m[col][row]);
		}
	}
	return seed;
}

} //namespace gl