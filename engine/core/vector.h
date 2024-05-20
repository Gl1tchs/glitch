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
	constexpr Vec(const std::initializer_list<T> init) :
			values(init.begin(), init.end()) {}
	constexpr Vec(const Vec& other) : values(other.values) {}
	constexpr Vec(Vec&& other) noexcept : values(std::move(other.values)) {}

	float dot(const Vec& other) const {
		float res = 0;
		for (int i = 0; i < TSize; i++) {
			res += values[i] * other[i];
		}
		return res;
	}

	T& operator[](size_t index) { return values[index]; }

	const T& operator[](size_t index) const { return values[index]; }

	constexpr Vec& operator=(const Vec& other) {
		values = other.values;
		return *this;
	}

	constexpr Vec operator+(const Vec& other) const {
		Vec res{};
		for (int i = 0; i < TSize; i++) {
			res[i] = values[i] + other[i];
		}
		return res;
	}

	constexpr Vec operator-(const Vec& other) const {
		Vec res{};
		for (int i = 0; i < TSize; i++) {
			res[i] = values[i] - other[i];
		}
		return res;
	}

	constexpr Vec operator*(const T& scalar) const {
		Vec res{};
		for (int i = 0; i < TSize; i++) {
			res[i] = values[i] * scalar;
		}
		return res;
	}

	constexpr Vec operator/(const T& scalar) const {
		Vec res{};
		for (int i = 0; i < TSize; i++) {
			res[i] = values[i] / scalar;
		}
		return res;
	}

	constexpr Vec& operator+=(const Vec& other) {
		*this = *this + other;
		return *this;
	}

	constexpr Vec& operator-=(const Vec& other) {
		*this = *this - other;
		return *this;
	}

	constexpr Vec& operator*=(const T& scalar) {
		*this = *this * scalar;
		return *this;
	}

	constexpr Vec& operator/=(const T& scalar) {
		*this = *this / scalar;
		return *this;
	}

	constexpr bool operator==(const Vec& other) const {
		for (int i = 0; i < TSize; i++) {
			if (values[i] != other[i]) {
				return false;
			}
		}
		return true;
	}
};

template <ArithmeticType T> struct Vec<T, 2> {
	T x;
	T y;

	constexpr Vec() : x(0), y(0) {}
	constexpr Vec(const T& val) : x(val), y(val) {}
	constexpr Vec(const T& x_val, const T& y_val) : x(x_val), y(y_val) {}
	constexpr Vec(const Vec& other) : x(other.x), y(other.y) {}
	constexpr Vec(Vec&& other) noexcept :
			x(std::move(other.x)), y(std::move(other.y)) {}

	float dot(const Vec& other) const { return x * other.x + y * other.y; }

	constexpr Vec& operator=(const Vec& other) {
		x = other.x;
		y = other.y;
		return *this;
	}

	constexpr Vec operator+(const Vec& other) const {
		return Vec(x + other.x, y + other.y);
	}

	constexpr Vec operator-(const Vec& other) const {
		return Vec(x - other.x, y - other.y);
	}

	constexpr Vec operator*(const T& scalar) const {
		return Vec(x * scalar, y * scalar);
	}

	constexpr Vec operator/(const T& scalar) const {
		return Vec(x / scalar, y / scalar);
	}

	constexpr Vec& operator+=(const Vec& other) {
		*this = *this + other;
		return *this;
	}

	constexpr Vec& operator-=(const Vec& other) {
		*this = *this - other;
		return *this;
	}

	constexpr Vec& operator*=(const T& scalar) {
		*this = *this * scalar;
		return *this;
	}

	constexpr Vec& operator/=(const T& scalar) {
		*this = *this / scalar;
		return *this;
	}

	constexpr bool operator==(const Vec& other) const {
		return x == other.x && y == other.y;
	}
};

using Vec2f = Vec<float, 2>;
using Vec2d = Vec<double, 2>;
using Vec2i = Vec<int, 2>;
using Vec2u = Vec<uint32_t, 2>;

template <ArithmeticType T> struct Vec<T, 3> {
	T x;
	T y;
	T z;

	constexpr Vec() : x(0), y(0), z(0) {}
	constexpr Vec(const T& val) : x(val), y(val), z(val) {}
	constexpr Vec(const T& x_val, const T& y_val, const T& z_val) :
			x(x_val), y(y_val), z(z_val) {}
	constexpr Vec(const Vec<T, 2>& vec2, const T& z_val) :
			x(vec2.x), y(vec2.y), z(z_val) {}
	constexpr Vec(const Vec& other) : x(other.x), y(other.y), z(other.z) {}
	constexpr Vec(Vec&& other) noexcept :
			x(std::move(other.x)),
			y(std::move(other.y)),
			z(std::move(other.z)) {}

	float dot(const Vec& other) const {
		return x * other.x + y * other.y + z * other.z;
	}

	constexpr Vec& operator=(const Vec& other) {
		x = other.x;
		y = other.y;
		z = other.z;
		return *this;
	}

	constexpr Vec operator+(const Vec& other) const {
		return Vec(x + other.x, y + other.y, z + other.z);
	}

	constexpr Vec operator-(const Vec& other) const {
		return Vec(x - other.x, y - other.y, z - other.z);
	}

	constexpr Vec operator*(const T& scalar) const {
		return Vec(x * scalar, y * scalar, z * scalar);
	}

	constexpr Vec operator/(const T& scalar) const {
		return Vec(x / scalar, y / scalar, z / scalar);
	}

	constexpr Vec& operator+=(const Vec& other) {
		*this = *this + other;
		return *this;
	}

	constexpr Vec& operator-=(const Vec& other) {
		*this = *this - other;
		return *this;
	}

	constexpr Vec& operator*=(const T& scalar) {
		*this = *this * scalar;
		return *this;
	}

	constexpr Vec& operator/=(const T& scalar) {
		*this = *this / scalar;
		return *this;
	}

	constexpr bool operator==(const Vec& other) const {
		return x == other.x && y == other.y && z == other.z;
	}
};

using Vec3f = Vec<float, 3>;
using Vec3d = Vec<double, 3>;
using Vec3i = Vec<int, 3>;
using Vec3u = Vec<uint32_t, 3>;

template <ArithmeticType T> struct Vec<T, 4> {
	T x;
	T y;
	T z;
	T w;

	constexpr Vec() : x(0), y(0), z(0), w(0) {}
	constexpr Vec(const T& val) : x(val), y(val), z(val), w(val) {}
	constexpr Vec(
			const T& x_val, const T& y_val, const T& z_val, const T& w_val) :
			x(x_val), y(y_val), z(z_val), w(w_val) {}
	constexpr Vec(const Vec<T, 2>& vec2, const T& z_val, const T& w_val) :
			x(vec2.x), y(vec2.y), z(z_val), w(w_val) {}
	constexpr Vec(const Vec<T, 4>& vec3, const T& w_val) :
			x(vec3.x), y(vec3.y), z(vec3.z), w(w_val) {}
	constexpr Vec(const Vec& other) :
			x(other.x), y(other.y), z(other.z), w(other.w) {}
	constexpr Vec(Vec&& other) noexcept :
			x(std::move(other.x)),
			y(std::move(other.y)),
			z(std::move(other.z)),
			w(std::move(other.w)) {}

	float dot(const Vec& other) const {
		return x * other.x + y * other.y + z * other.z + w * other.w;
	}

	constexpr Vec& operator=(const Vec& other) {
		x = other.x;
		y = other.y;
		z = other.z;
		w = other.w;
		return *this;
	}

	constexpr Vec operator+(const Vec& other) const {
		return Vec(x + other.x, y + other.y, z + other.z, w + other.w);
	}

	constexpr Vec operator-(const Vec& other) const {
		return Vec(x - other.x, y - other.y, z - other.z, w - other.w);
	}

	constexpr Vec operator*(const T& scalar) const {
		return Vec(x * scalar, y * scalar, z * scalar, w * scalar);
	}

	constexpr Vec operator/(const T& scalar) const {
		return Vec(x / scalar, y / scalar, z / scalar, w * scalar);
	}

	constexpr Vec& operator+=(const Vec& other) {
		*this = *this + other;
		return *this;
	}

	constexpr Vec& operator-=(const Vec& other) {
		*this = *this - other;
		return *this;
	}

	constexpr Vec& operator*=(const T& scalar) {
		*this = *this * scalar;
		return *this;
	}

	constexpr Vec& operator/=(const T& scalar) {
		*this = *this / scalar;
		return *this;
	}

	constexpr bool operator==(const Vec& other) const {
		return x == other.x && y == other.y && z == other.z;
	}
};

using Vec4f = Vec<float, 4>;
using Vec4d = Vec<double, 4>;
using Vec4i = Vec<int, 4>;
using Vec4u = Vec<uint32_t, 4>;
