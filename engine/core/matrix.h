#pragma once

#include "core/vector.h"

#include <immintrin.h>

template <ArithmeticType T, size_t TColumns, size_t TRows> struct Mat {
	std::array<std::array<T, TColumns>, TRows> columns;

	constexpr Mat() {
		// not-safe function but we are ensuring the type is arithmetic
		memset(&columns[0][0], 0, sizeof(columns));
	}

	constexpr Mat(T value) {
		for (int c = 0; c < TColumns; c++) {
			for (int r = 0; r < TRows; r++) {
				if (c == r) {
					columns[c][r] = value;
				} else {
					columns[c][r] = static_cast<T>(0);
				}
			}
		}
	}

	constexpr Mat(std::initializer_list<std::initializer_list<T>> init) {
		GL_ASSERT(init.size() == TColumns);

		size_t column = 0;
		for (const auto& rows : init) {
			GL_ASSERT(rows.size() == TRows);

			size_t row = 0;
			for (const auto& element : rows) {
				columns[column][row] = element;
				row++;
			}
			column++;
		}
	}

	constexpr std::array<T, TColumns>& operator[](size_t index) {
		return columns[index];
	}

	constexpr const std::array<T, TColumns>& operator[](size_t index) const {
		return columns[index];
	}

	constexpr Mat operator+(const Mat& other) const {
		Mat result;
		for (size_t i = 0; i < TRows; ++i) {
			for (size_t j = 0; j < TColumns; j += 8) {
				__m256 vec1 = _mm256_loadu_ps(&columns[i][j]);
				__m256 vec2 = _mm256_loadu_ps(&other.columns[i][j]);
				__m256 vec_result = _mm256_add_ps(vec1, vec2);
				_mm256_storeu_ps(&result.columns[i][j], vec_result);
			}
		}
		return result;
	}

	constexpr Mat operator-(const Mat& other) const {
		Mat result;
		for (size_t i = 0; i < TRows; ++i) {
			for (size_t j = 0; j < TColumns; j += 8) {
				__m256 vec1 = _mm256_loadu_ps(&columns[i][j]);
				__m256 vec2 = _mm256_loadu_ps(&other.columns[i][j]);
				__m256 vec_result = _mm256_sub_ps(vec1, vec2);
				_mm256_storeu_ps(&result.columns[i][j], vec_result);
			}
		}
		return result;
	}

	constexpr Mat operator*(const T& scalar) const {
		Mat result;
		__m256 scalar_vec = _mm256_set1_ps(scalar);
		for (size_t i = 0; i < TRows; ++i) {
			for (size_t j = 0; j < TColumns; j += 8) {
				__m256 vec = _mm256_loadu_ps(&columns[i][j]);
				__m256 vec_result = _mm256_mul_ps(vec, scalar_vec);
				_mm256_storeu_ps(&result.columns[i][j], vec_result);
			}
		}
		return result;
	}

	/*
	constexpr Mat operator*(const Mat<T, TRows, TColumns> other) const {

	}
	*/

	constexpr Mat operator/(const T& scalar) const {
		GL_ASSERT(scalar != 0);

		Mat result;
		__m256 scalar_vec = _mm256_set1_ps(scalar);
		for (size_t i = 0; i < TRows; ++i) {
			for (size_t j = 0; j < TColumns; j += 8) {
				__m256 vec = _mm256_loadu_ps(&columns[i][j]);
				__m256 vec_result = _mm256_div_ps(vec, scalar_vec);
				_mm256_storeu_ps(&result.columns[i][j], vec_result);
			}
		}
		return result;
	}

	constexpr Mat& operator+=(const Mat& other) {
		*this = *this + other;
		return *this;
	}

	constexpr Mat& operator-=(const Mat& other) {
		*this = *this - other;
		return *this;
	}

	constexpr Mat& operator*=(const T& other) {
		*this = *this * other;
		return *this;
	}

	constexpr Mat& operator/=(const T& other) {
		*this = *this / other;
		return *this;
	}

	constexpr bool operator==(const Mat& other) const {
		for (int c = 0; c < TColumns; c++) {
			for (int r = 0; r < TRows; r++) {
				if (columns[c][r] != other[c][r]) {
					return false;
				}
			}
		}
		return true;
	}
};
