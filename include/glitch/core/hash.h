/**
 * @file hash.h
 */

#pragma once

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

template <> inline size_t hash64(const std::vector<int>& p_value) {
	size_t seed = 0;
	for (size_t i = 0; i < p_value.size(); i++) {
		hash_combine(seed, p_value[i]);
	}
	return seed;
}