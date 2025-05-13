#include <doctest/doctest.h>

#include "glitch/core/math/matrix.h"

TEST_CASE("Mat4f default constructor") {
	Mat4f mat;
	CHECK(mat[0][0] == 0);
	CHECK(mat[1][1] == 0);
	CHECK(mat[2][2] == 0);
	CHECK(mat[3][3] == 0);
}

TEST_CASE("Mat4f scalar constructor") {
	Mat4f mat(5.0f);

	CHECK(mat[0][0] == 5.0f);
	CHECK(mat[1][1] == 5.0f);
	CHECK(mat[2][2] == 5.0f);
	CHECK(mat[3][3] == 5.0f);

	// other elements should be 0
	CHECK(mat[0][1] == 0);
	CHECK(mat[1][0] == 0);
}

TEST_CASE("Mat4f matrix multiplication") {
	Mat4f mat1{ { Vec4f{ 1, 2, 3, 4 } }, { Vec4f{ 5, 6, 7, 8 } },
		{ Vec4f{ 9, 10, 11, 12 } }, { Vec4f{ 13, 14, 15, 16 } } };

	Mat4f mat2{ { Vec4f{ 16, 15, 14, 13 } }, { Vec4f{ 12, 11, 10, 9 } },
		{ Vec4f{ 8, 7, 6, 5 } }, { Vec4f{ 4, 3, 2, 1 } } };

	Mat4f result = mat1 * mat2;

	CHECK(result[0][0] == 386);
	CHECK(result[0][1] == 444);
	CHECK(result[0][2] == 502);
	CHECK(result[0][3] == 560);
	CHECK(result[3][0] == 50);
	CHECK(result[3][1] == 60);
}

TEST_CASE("Mat4f matrix transpose") {
	Mat4f mat{ { Vec4f{ 1, 2, 3, 4 } }, { Vec4f{ 5, 6, 7, 8 } },
		{ Vec4f{ 9, 10, 11, 12 } }, { Vec4f{ 13, 14, 15, 16 } } };
	Mat4f transposed = mat.transpose();

	CHECK(transposed[0][0] == 1);
	CHECK(transposed[1][0] == 2);
	CHECK(transposed[2][0] == 3);
	CHECK(transposed[3][0] == 4);
	CHECK(transposed[0][3] == 13);
	CHECK(transposed[3][3] == 16);
}

TEST_CASE("Mat4f matrix equality") {
	Mat4f mat1{ { Vec4f{ 1, 2, 3, 4 } }, { Vec4f{ 5, 6, 7, 8 } },
		{ Vec4f{ 9, 10, 11, 12 } }, { Vec4f{ 13, 14, 15, 16 } } };
	Mat4f mat2{ { Vec4f{ 1, 2, 3, 4 } }, { Vec4f{ 5, 6, 7, 8 } },
		{ Vec4f{ 9, 10, 11, 12 } }, { Vec4f{ 13, 14, 15, 16 } } };

	CHECK(mat1 == mat2);

	mat2[0][0] = 999.0f;

	CHECK(mat1 != mat2);
}
