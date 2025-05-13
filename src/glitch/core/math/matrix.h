#pragma once

#include "glitch/core/debug/profiling.h"
#include "glitch/core/math/vector.h"

template <ArithmeticType T, size_t TColums, size_t TRows> struct Mat;

template <> struct Mat<float, 4, 4> {
	Vec4f m[4];

	constexpr Mat() : m{ {}, {}, {}, {} } {}
	constexpr Mat(float p_scalar) {
		m[0] = Vec4f(p_scalar, 0, 0, 0);
		m[1] = Vec4f(0, p_scalar, 0, 0);
		m[2] = Vec4f(0, 0, p_scalar, 0);
		m[3] = Vec4f(0, 0, 0, p_scalar);
	}
	constexpr Mat(std::initializer_list<Vec4f> init) {
		auto it = init.begin();
		for (int i = 0; i < 4; ++i) {
			m[i][0] = (*it)[0];
			m[i][1] = (*it)[1];
			m[i][2] = (*it)[2];
			m[i][3] = (*it)[3];
			++it;
		}
	}
	constexpr Mat(const Mat& p_other) :
			m{ p_other.m[0], p_other.m[1], p_other.m[2], p_other.m[3] } {}
	constexpr Mat(Mat&& p_other) noexcept :
			m{ std::move(p_other.m[0]), std::move(p_other.m[1]),
				std::move(p_other.m[2]), std::move(p_other.m[3]) } {}

	Mat& operator=(const Mat& p_other) {
		if (this != &p_other) {
			m[0] = p_other.m[0];
			m[1] = p_other.m[1];
			m[2] = p_other.m[2];
			m[3] = p_other.m[3];
		}
		return *this;
	}

	Mat& operator=(Mat&& p_other) noexcept {
		if (this != &p_other) {
			m[0] = std::move(p_other.m[0]);
			m[1] = std::move(p_other.m[1]);
			m[2] = std::move(p_other.m[2]);
			m[3] = std::move(p_other.m[3]);
		}
		return *this;
	}

	Vec4f& operator[](size_t p_index) { return m[p_index]; }
	const Vec4f& operator[](size_t p_index) const { return m[p_index]; }

	Mat transpose() const {
		Mat result;
		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j) {
				result.m[i][j] = m[j][i];
			}
		}
		return result;
	}

	Mat operator*(const Mat& p_other) const {
		GL_PROFILE_SCOPE;

		Mat result;
		for (int col = 0; col < 4; ++col) {
			for (int row = 0; row < 4; ++row) {
				result.m[col][row] = m[0][row] * p_other.m[col][0] +
						m[1][row] * p_other.m[col][1] +
						m[2][row] * p_other.m[col][2] +
						m[3][row] * p_other.m[col][3];
			}
		}

		return result;
	}

	bool operator==(const Mat& other) const {
		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j) {
				if (m[i][j] != other.m[i][j]) {
					return false;
				}
			}
		}
		return true;
	}
};

typedef Mat<float, 4, 4> Mat4f;
