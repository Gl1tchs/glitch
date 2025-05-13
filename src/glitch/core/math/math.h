/**
 * @file utils.h
 */

#pragma once

#include "glitch/core/math/vector.h"

namespace gmath {

constexpr float RADIAN = std::numbers::pi / 180.0f;

template <ArithmeticType T> constexpr T to_radians(const T& p_val) {
	return p_val * RADIAN;
}

template <ArithmeticType T, size_t TSize>
constexpr Vec<T, TSize> to_radians(const Vec<T, TSize>& p_vec) {
	return p_vec * gmath::RADIAN;
}

} //namespace gmath