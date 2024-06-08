#pragma once

template <typename T> class BitField {
	int64_t value = 0;

public:
	inline BitField<T>& set_flag(T p_flag) {
		value |= (int64_t)p_flag;
		return *this;
	}
	inline bool has_flag(T p_flag) const { return value & (int64_t)p_flag; }
	inline bool is_empty() const { return value == 0; }
	inline void clear_flag(T p_flag) { value &= ~(int64_t)p_flag; }
	inline void clear() { value = 0; }
	inline constexpr BitField() = default;
	inline constexpr BitField(int64_t p_value) { value = p_value; }
	inline constexpr BitField(T p_value) { value = (int64_t)p_value; }
	inline operator int64_t() const { return value; }
	inline BitField<T> operator^(const BitField<T>& p_b) const {
		return BitField<T>(value ^ p_b.value);
	}
};
