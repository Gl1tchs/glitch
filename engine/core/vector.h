#pragma once

#include <type_traits>

template <typename T>
concept ArithmeticType = std::is_arithmetic_v<T>;

template <ArithmeticType T> using Scalar = T;

template <ArithmeticType T> struct Vector2 {
	T x;
	T y;

	constexpr Vector2() : x(0), y(0) {}
	constexpr Vector2(const T& val) : x(val), y(val) {}
	constexpr Vector2(const T& x_val, const T& y_val) : x(x_val), y(y_val) {}
	constexpr Vector2(const Vector2& other) : x(other.x), y(other.y) {}
	constexpr Vector2(Vector2&& other) noexcept :
			x(std::move(other.x)), y(std::move(other.y)) {}

	constexpr Vector2& operator=(const Vector2& other) {
		x = other.x;
		y = other.y;
		return *this;
	}

	constexpr Vector2 operator+(const Vector2& other) const {
		return Vector2(x + other.x, y + other.y);
	}

	constexpr Vector2 operator-(const Vector2& other) const {
		return Vector2(x - other.x, y - other.y);
	}

	constexpr Vector2 operator*(const T& scalar) const {
		return Vector2(x * scalar, y * scalar);
	}

	constexpr Vector2 operator/(const T& scalar) const {
		static_assert(scalar != 0 || std::abs(scalar) - 0.00001f > 0.0f);
		return Vector2(x / scalar, y / scalar);
	}
};

using Vector2f = Vector2<float>;
using Vector2d = Vector2<double>;
using Vector2i = Vector2<int>;
using Vector2u = Vector2<unsigned int>;

template <ArithmeticType T> struct Vector3 {
	T x;
	T y;
	T z;

	constexpr Vector3() : x(0), y(0), z(0) {}
	constexpr Vector3(const T& val) : x(val), y(val), z(val) {}
	constexpr Vector3(const T& x_val, const T& y_val, const T& z_val) :
			x(x_val), y(y_val), z(z_val) {}
	constexpr Vector3(const Vector2<T>& vec2, const T& z_val) :
			x(vec2.x), y(vec2.y), z(z_val) {}
	constexpr Vector3(const Vector3& other) :
			x(other.x), y(other.y), z(other.z) {}
	constexpr Vector3(Vector3&& other) noexcept :
			x(std::move(other.x)),
			y(std::move(other.y)),
			z(std::move(other.z)) {}

	constexpr Vector3& operator=(const Vector3& other) {
		x = other.x;
		y = other.y;
		z = other.z;
		return *this;
	}

	constexpr Vector3 operator+(const Vector3& other) const {
		return Vector3(x + other.x, y + other.y, z + other.z);
	}

	constexpr Vector3 operator-(const Vector3& other) const {
		return Vector3(x - other.x, y - other.y, z - other.z);
	}

	constexpr Vector3 operator*(const T& scalar) const {
		return Vector3(x * scalar, y * scalar, z * scalar);
	}

	constexpr Vector3 operator/(const T& scalar) const {
		static_assert(scalar != 0 || std::abs(scalar) - 0.00001f > 0.0f);
		return Vector3(x / scalar, y / scalar, z / scalar);
	}
};

using Vector3f = Vector3<float>;
using Vector3d = Vector3<double>;
using Vector3i = Vector3<int>;
using Vector3u = Vector3<unsigned int>;

template <ArithmeticType T> struct Vector4 {
	T x;
	T y;
	T z;
	T w;

	constexpr Vector4() : x(0), y(0), z(0), w(0) {}
	constexpr Vector4(const T& val) : x(val), y(val), z(val), w(val) {}
	constexpr Vector4(
			const T& x_val, const T& y_val, const T& z_val, const T& w_val) :
			x(x_val), y(y_val), z(z_val), w(w_val) {}
	constexpr Vector4(const Vector2<T>& vec2, const T& z_val, const T& w_val) :
			x(vec2.x), y(vec2.y), z(z_val), w(w_val) {}
	constexpr Vector4(const Vector3<T>& vec3, const T& w_val) :
			x(vec3.x), y(vec3.y), z(vec3.z), w(w_val) {}
	constexpr Vector4(const Vector4& other) :
			x(other.x), y(other.y), z(other.z), w(other.w) {}
	constexpr Vector4(Vector4&& other) noexcept :
			x(std::move(other.x)),
			y(std::move(other.y)),
			z(std::move(other.z)),
			w(std::move(other.w)) {}

	constexpr Vector4& operator=(const Vector4& other) {
		x = other.x;
		y = other.y;
		z = other.z;
		w = other.w;
		return *this;
	}

	constexpr Vector4 operator+(const Vector4& other) const {
		return Vector4(x + other.x, y + other.y, z + other.z, w + other.w);
	}

	constexpr Vector4 operator-(const Vector4& other) const {
		return Vector4(x - other.x, y - other.y, z - other.z, w - other.w);
	}

	constexpr Vector4 operator*(const T& scalar) const {
		return Vector4(x * scalar, y * scalar, z * scalar, w * scalar);
	}

	constexpr Vector4 operator/(const T& scalar) const {
		static_assert(scalar != 0 || std::abs(scalar) - 0.00001f > 0.0f);
		return Vector4(x / scalar, y / scalar, z / scalar, w * scalar);
	}
};

using Vector4f = Vector4<float>;
using Vector4d = Vector4<double>;
using Vector4i = Vector4<int>;
using Vector4u = Vector4<unsigned int>;
