/**
 * @file bit_field.h
 */

#pragma once

namespace gl {

/**
 * Class that makes flag functionality a little bit simpler.
 */
template <typename T> class BitField {
public:
	constexpr BitField() = default;
	constexpr BitField(int64_t p_value) { value = p_value; }
	constexpr BitField(T p_value) { value = (int64_t)p_value; }

	BitField<T>& set_flag(T p_flag) {
		value |= (int64_t)p_flag;
		return *this;
	}

	bool has_flag(T p_flag) const { return value & (int64_t)p_flag; }

	bool is_empty() const { return value == 0; }

	void clear_flag(T p_flag) { value &= ~(int64_t)p_flag; }

	void clear() { value = 0; }

	operator int64_t() const { return value; }

	BitField<T> operator^(const BitField<T>& p_b) const {
		return BitField<T>(value ^ p_b.value);
	}

private:
	int64_t value = 0;
};

} //namespace gl