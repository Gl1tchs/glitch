/**
 * @file hash.h
 */

#pragma once

#include "glitch/core/core.h"

#include <cstdint>
#include <functional>
#include <vector>

namespace gl {

template <typename T> inline void hash_combine(std::size_t& seed, T const& value) {
	seed ^= std::hash<T>()(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template <typename T> inline size_t hash64(const T& value) {
	size_t seed = 0;
	hash_combine(seed, value);
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

template <typename T> inline size_t hash64(const std::vector<T>& value) {
	size_t seed = 0;
	for (size_t i = 0; i < value.size(); i++) {
		hash_combine(seed, value[i]);
	}
	return seed;
}

template <> inline size_t hash64(const Vec3f& v) {
	size_t seed = 0;
	hash_combine(seed, v.x);
	hash_combine(seed, v.y);
	hash_combine(seed, v.z);
	return seed;
}

template <> inline size_t hash64(const Vec4f& v) {
	size_t seed = 0;
	hash_combine(seed, v.x);
	hash_combine(seed, v.y);
	hash_combine(seed, v.z);
	hash_combine(seed, v.w);
	return seed;
}

template <> inline size_t hash64(const Mat4& m) {
	size_t seed = 0;
	for (int col = 0; col < 4; ++col) {
		for (int row = 0; row < 4; ++row) {
			hash_combine(seed, m.cols[col][row]);
		}
	}
	return seed;
}

} //namespace gl