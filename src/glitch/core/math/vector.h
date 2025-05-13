/**
 * @file vector.h
 */

#pragma once

#include <type_traits>

template <typename T>
concept ArithmeticType = std::is_arithmetic_v<T>;

template <ArithmeticType T, size_t TSize> struct Vec {
	std::array<T, TSize> values;

	constexpr Vec() { std::fill(values.begin(), values.end(), 0.0f); }
	constexpr Vec(const T& val) {
		std::fill(values.begin(), values.end(), val);
	}
	constexpr Vec(const std::initializer_list<T> init) {
		if (init.size() != TSize) {
			throw std::runtime_error(
					"Initializer list size does not match vector size");
		}
		std::copy(init.begin(), init.end(), values.begin());
	}
	constexpr Vec(const Vec& p_other) : values(p_other.values) {}
	constexpr Vec(Vec&& p_other) noexcept : values(std::move(p_other.values)) {}

	float magnitude() const {
		float v = 0;
		for (int i = 0; i < TSize; i++) {
			v += values[i] * values[i];
		}
		return sqrtf(v);
	}

	Vec normalize() const { return *this / magnitude(); }

	float dot(const Vec& p_other) const {
		float res = 0;
		for (int i = 0; i < TSize; i++) {
			res += values[i] * p_other[i];
		}
		return res;
	}

	T& operator[](size_t p_index) { return values[p_index]; }

	const T& operator[](size_t p_index) const { return values[p_index]; }

	constexpr Vec& operator=(const Vec& p_other) {
		values = p_other.values;
		return *this;
	}

	constexpr Vec operator+(const Vec& p_other) const {
		Vec res{};
		for (int i = 0; i < TSize; i++) {
			res[i] = values[i] + p_other[i];
		}
		return res;
	}

	constexpr Vec operator-(const Vec& p_other) const {
		Vec res{};
		for (int i = 0; i < TSize; i++) {
			res[i] = values[i] - p_other[i];
		}
		return res;
	}

	constexpr Vec operator*(const T& p_scalar) const {
		Vec res{};
		for (int i = 0; i < TSize; i++) {
			res[i] = values[i] * p_scalar;
		}
		return res;
	}

	constexpr Vec operator/(const T& p_scalar) const {
		Vec res{};
		for (int i = 0; i < TSize; i++) {
			res[i] = values[i] / p_scalar;
		}
		return res;
	}

	constexpr Vec& operator+=(const Vec& p_other) {
		*this = *this + p_other;
		return *this;
	}

	constexpr Vec& operator-=(const Vec& p_other) {
		*this = *this - p_other;
		return *this;
	}

	constexpr Vec& operator*=(const T& p_scalar) {
		*this = *this * p_scalar;
		return *this;
	}

	constexpr Vec& operator/=(const T& p_scalar) {
		*this = *this / p_scalar;
		return *this;
	}

	constexpr bool operator==(const Vec& p_other) const {
		for (int i = 0; i < TSize; i++) {
			if (values[i] != p_other[i]) {
				return false;
			}
		}
		return true;
	}
};

template <ArithmeticType T> struct Vec2 {
	T x;
	T y;

	constexpr Vec2() : x(0), y(0) {}
	constexpr Vec2(T p_val) : x(p_val), y(p_val) {}
	constexpr Vec2(T p_x, T p_y) : x(p_x), y(p_y) {}

	constexpr Vec2(const Vec2& p_other) : x(p_other.x), y(p_other.y) {}
	constexpr Vec2(Vec2&& p_other) :
			x(std::move(p_other.x)), y(std::move(p_other.y)) {}

	constexpr Vec2& operator=(const Vec2& p_other) {
		x = p_other.x;
		y = p_other.y;
		return *this;
	}

	float magnitude() const { return sqrtf(x * x + y * y); }

	Vec2 normalize() const { return *this / magnitude(); }

	float dot(const Vec2& p_other) const {
		return x * p_other.x + y * p_other.y;
	}

	T& operator[](size_t p_index) {
		switch (p_index) {
			case 0:
				return x;
			case 1:
				return y;
			default:
				throw std::out_of_range("Index out of range");
		}
	}

	const T& operator[](size_t p_index) const {
		switch (p_index) {
			case 0:
				return x;
			case 1:
				return y;
			default:
				throw std::out_of_range("Index out of range");
		}
	}

	constexpr Vec2 operator+(const Vec2& p_other) const {
		return Vec2(x + p_other.x, y + p_other.y);
	}

	constexpr Vec2 operator-(const Vec2& p_other) const {
		return Vec2(x - p_other.x, y - p_other.y);
	}

	constexpr Vec2 operator*(const T& p_scalar) const {
		return Vec2(x * p_scalar, y * p_scalar);
	}

	constexpr Vec2 operator*(const Vec2& vec2) const {
		return Vec2(x * vec2.x, y * vec2.y);
	}

	constexpr Vec2 operator/(const T& p_scalar) const {
		return Vec2(x / p_scalar, y / p_scalar);
	}

	constexpr Vec2& operator+=(const Vec2& p_other) {
		*this = *this + p_other;
		return *this;
	}

	constexpr Vec2& operator-=(const Vec2& p_other) {
		*this = *this - p_other;
		return *this;
	}

	constexpr Vec2& operator*=(const T& p_scalar) {
		*this = *this * p_scalar;
		return *this;
	}

	constexpr Vec2& operator*=(const Vec2& p_other) {
		*this = *this * p_other;
		return *this;
	}

	constexpr Vec2& operator/=(const T& p_scalar) {
		*this = *this / p_scalar;
		return *this;
	}

	constexpr bool operator==(const Vec2& p_other) const {
		return x == p_other.x && y == p_other.y;
	}
};

using Vec2f = Vec2<float>;
using Vec2d = Vec2<double>;
using Vec2i = Vec2<int>;
using Vec2u = Vec2<uint32_t>;

template <ArithmeticType T> struct Vec3 {
	T x;
	T y;
	T z;

	constexpr Vec3() : x(0), y(0), z(0) {}
	constexpr Vec3(T p_val) : x(p_val), y(p_val), z(p_val) {}
	constexpr Vec3(T p_x, T p_y, T p_z) : x(p_x), y(p_y), z(p_z) {}

	constexpr Vec3(const Vec3& p_other) :
			x(p_other.x), y(p_other.y), z(p_other.z) {}
	constexpr Vec3(Vec3&& p_other) :
			x(std::move(p_other.x)),
			y(std::move(p_other.y)),
			z(std::move(p_other.z)) {}

	constexpr Vec3& operator=(const Vec3& p_other) {
		x = p_other.x;
		y = p_other.y;
		z = p_other.z;
		return *this;
	}

	float magnitude() const { return sqrtf(x * x + y * y + z * z); }

	Vec3 normalize() const { return *this / magnitude(); }

	float dot(const Vec3& p_other) const {
		return x * p_other.x + y * p_other.y + z * p_other.z;
	}

	Vec3 cross(const Vec3& p_other) const {
		return Vec3(y * p_other.z - z * p_other.y,
				z * p_other.x - x * p_other.z, x * p_other.y - y * p_other.x);
	}

	T& operator[](size_t p_index) {
		switch (p_index) {
			case 0:
				return x;
			case 1:
				return y;
			case 2:
				return z;
			default:
				throw std::out_of_range("Index out of range");
		}
	}

	const T& operator[](size_t p_index) const {
		switch (p_index) {
			case 0:
				return x;
			case 1:
				return y;
			case 2:
				return z;
			default:
				throw std::out_of_range("Index out of range");
		}
	}

	constexpr Vec3 operator+(const Vec3& p_other) const {
		return Vec3(x + p_other.x, y + p_other.y, z + p_other.z);
	}

	constexpr Vec3 operator-(const Vec3& p_other) const {
		return Vec3(x - p_other.x, y - p_other.y, z - p_other.z);
	}

	constexpr Vec3 operator*(const T& p_scalar) const {
		return Vec3(x * p_scalar, y * p_scalar, z * p_scalar);
	}

	constexpr Vec3 operator*(const Vec3& p_other) const {
		return Vec3(x * p_other.x, y * p_other.y, z * p_other.z);
	}

	constexpr Vec3 operator/(const T& p_scalar) const {
		return Vec3(x / p_scalar, y / p_scalar, z / p_scalar);
	}

	constexpr Vec3& operator+=(const Vec3& p_other) {
		*this = *this + p_other;
		return *this;
	}

	constexpr Vec3& operator-=(const Vec3& p_other) {
		*this = *this - p_other;
		return *this;
	}

	constexpr Vec3& operator*=(const T& p_scalar) {
		*this = *this * p_scalar;
		return *this;
	}

	constexpr Vec3& operator*=(const Vec3& p_other) {
		*this = *this * p_other;
		return *this;
	}

	constexpr Vec3& operator/=(const T& p_scalar) {
		*this = *this / p_scalar;
		return *this;
	}

	constexpr bool operator==(const Vec3& p_other) const {
		return x == p_other.x && y == p_other.y && z == p_other.z;
	}
};

using Vec3f = Vec3<float>;
using Vec3d = Vec3<double>;
using Vec3i = Vec3<int>;
using Vec3u = Vec3<uint32_t>;

template <ArithmeticType T> struct Vec4 {
	T x;
	T y;
	T z;
	T w;

	constexpr Vec4() : x(0), y(0), z(0), w(0) {}
	constexpr Vec4(const T& p_val) : x(p_val), y(p_val), z(p_val), w(p_val) {}
	constexpr Vec4(const T& p_x, const T& p_y, const T& p_z, const T& p_w) :
			x(p_x), y(p_y), z(p_z), w(p_w) {}

	constexpr Vec4(const Vec4& p_other) :
			x(p_other.x), y(p_other.y), z(p_other.z), w(p_other.w) {}
	constexpr Vec4(Vec4&& p_other) :
			x(std::move(p_other.x)),
			y(std::move(p_other.y)),
			z(std::move(p_other.z)),
			w(std::move(p_other.w)) {}

	constexpr Vec4& operator=(const Vec4& p_other) {
		x = p_other.x;
		y = p_other.y;
		z = p_other.z;
		w = p_other.w;
		return *this;
	}

	float magnitude() const { return sqrtf(x * x + y * y + z * z + w * w); }

	Vec4 normalize() const { return *this / magnitude(); }

	float dot(const Vec4& p_other) const {
		return x * p_other.x + y * p_other.y + z * p_other.z + w * p_other.w;
	}

	T& operator[](size_t p_index) {
		switch (p_index) {
			case 0:
				return x;
			case 1:
				return y;
			case 2:
				return z;
			case 3:
				return w;
			default:
				throw std::out_of_range("Index out of range");
		}
	}

	const T& operator[](size_t p_index) const {
		switch (p_index) {
			case 0:
				return x;
			case 1:
				return y;
			case 2:
				return z;
			case 3:
				return w;
			default:
				throw std::out_of_range("Index out of range");
		}
	}

	constexpr Vec4 operator+(const Vec4& p_other) const {
		return Vec4(x + p_other.x, y + p_other.y, z + p_other.z, w + p_other.w);
	}

	constexpr Vec4 operator-(const Vec4& p_other) const {
		return Vec4(x - p_other.x, y - p_other.y, z - p_other.z, w - p_other.w);
	}

	constexpr Vec4 operator*(const T& p_scalar) const {
		return Vec4(x * p_scalar, y * p_scalar, z * p_scalar, w * p_scalar);
	}

	constexpr Vec4 operator*(const Vec4& p_other) const {
		return Vec4(x * p_other.x, y * p_other.y, z * p_other.z, w * p_other.w);
	}

	constexpr Vec4 operator/(const T& p_scalar) const {
		return Vec4(x / p_scalar, y / p_scalar, z / p_scalar, w * p_scalar);
	}

	constexpr Vec4& operator+=(const Vec4& p_other) {
		*this = *this + p_other;
		return *this;
	}

	constexpr Vec4& operator-=(const Vec4& p_other) {
		*this = *this - p_other;
		return *this;
	}

	constexpr Vec4& operator*=(const T& p_scalar) {
		*this = *this * p_scalar;
		return *this;
	}

	constexpr Vec4& operator*=(const Vec4& p_other) {
		*this = *this * p_other;
		return *this;
	}

	constexpr Vec4& operator/=(const T& p_scalar) {
		*this = *this / p_scalar;
		return *this;
	}

	constexpr bool operator==(const Vec4& p_other) const {
		return x == p_other.x && y == p_other.y && z == p_other.z;
	}
};

using Vec4f = Vec4<float>;
using Vec4d = Vec4<double>;
using Vec4i = Vec4<int>;
using Vec4u = Vec4<uint32_t>;

// this handles p_scalar * Vec<T, TSize>
template <ArithmeticType T, size_t TSize>
constexpr Vec<T, TSize> operator*(
		const T& p_scalar, const Vec<T, TSize>& p_vec) {
	return p_vec * p_scalar; // reuse Vec * p_scalar
}
