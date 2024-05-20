#include <catch2/catch_all.hpp>

#include "core/matrix.h"

TEST_CASE("Mat Constructors", "[mat]") {
	SECTION("Default Constructor") {
		Mat<float, 3, 3> m;
		for (size_t i = 0; i < 3; ++i) {
			for (size_t j = 0; j < 3; ++j) {
				REQUIRE(m[i][j] == 0.0f);
			}
		}
	}

	SECTION("Single Value Constructor") {
		Mat<float, 3, 3> m(5.0f);
		for (size_t i = 0; i < 3; ++i) {
			for (size_t j = 0; j < 3; ++j) {
				if (i == j) {
					REQUIRE(m[i][j] == 5.0f);
				} else {
					REQUIRE(m[i][j] == 0.0f);
				}
			}
		}
	}
}

TEST_CASE("Mat Arithmetic Operators", "[mat]") {
	SECTION("Addition") {
		Mat<float, 3, 3> m1(1.0f);
		Mat<float, 3, 3> m2(2.0f);
		Mat<float, 3, 3> result = m1 + m2;
		Mat<float, 3, 3> expected = {
			{ 3.0f, 0.0f, 0.0f },
			{ 0.0f, 3.0f, 0.0f },
			{ 0.0f, 0.0f, 3.0f },
		};

		REQUIRE(result == expected);
	}

	SECTION("Subtraction") {
		Mat<float, 3, 3> m1(3.0f);
		Mat<float, 3, 3> m2(1.0f);
		Mat<float, 3, 3> result = m1 - m2;
		Mat<float, 3, 3> expected = {
			{ 2.0f, 0.0f, 0.0f },
			{ 0.0f, 2.0f, 0.0f },
			{ 0.0f, 0.0f, 2.0f },
		};

		REQUIRE(result == expected);
	}

	SECTION("Multiplication by scalar") {
		Mat<float, 3, 3> m(2.0f);
		Mat<float, 3, 3> result = m * 3.0f;
		Mat<float, 3, 3> expected = {
			{ 6.0f, 0.0f, 0.0f },
			{ 0.0f, 6.0f, 0.0f },
			{ 0.0f, 0.0f, 6.0f },
		};

		REQUIRE(result == expected);
	}

	SECTION("Division by scalar") {
		Mat<float, 3, 3> m(6.0f);
		Mat<float, 3, 3> result = m / 2.0f;
		Mat<float, 3, 3> expected = {
			{ 3.0f, 0.0f, 0.0f },
			{ 0.0f, 3.0f, 0.0f },
			{ 0.0f, 0.0f, 3.0f },
		};

		REQUIRE(result == expected);
	}
}

TEST_CASE("Mat Compound Assignment Operators", "[mat]") {
	SECTION("Addition Assignment") {
		Mat<float, 3, 3> m1(1.0f);
		Mat<float, 3, 3> m2(2.0f);
		m1 += m2;

		Mat<float, 3, 3> expected = {
			{ 3.0f, 0.0f, 0.0f },
			{ 0.0f, 3.0f, 0.0f },
			{ 0.0f, 0.0f, 3.0f },
		};

		REQUIRE(m1 == expected);
	}

	SECTION("Subtraction Assignment") {
		Mat<float, 3, 3> m1(3.0f);
		Mat<float, 3, 3> m2(1.0f);
		m1 -= m2;

		Mat<float, 3, 3> expected = {
			{ 2.0f, 0.0f, 0.0f },
			{ 0.0f, 2.0f, 0.0f },
			{ 0.0f, 0.0f, 2.0f },
		};

		REQUIRE(m1 == expected);
	}

	SECTION("Multiplication Assignment") {
		Mat<float, 3, 3> m(2.0f);
		m *= 3.0f;

		Mat<float, 3, 3> expected = {
			{ 6.0f, 0.0f, 0.0f },
			{ 0.0f, 6.0f, 0.0f },
			{ 0.0f, 0.0f, 6.0f },
		};

		REQUIRE(m == expected);
	}

	SECTION("Division Assignment") {
		Mat<float, 3, 3> m(6.0f);
		m /= 2.0f;

		Mat<float, 3, 3> expected = {
			{ 3.0f, 0.0f, 0.0f },
			{ 0.0f, 3.0f, 0.0f },
			{ 0.0f, 0.0f, 3.0f },
		};

		REQUIRE(m == expected);
	}
}

TEST_CASE("Mat Comparison Operators", "[mat]") {
	SECTION("Equality") {
		Mat<float, 3, 3> m1(2.0f);
		Mat<float, 3, 3> m2(2.0f);
		Mat<float, 3, 3> m3(3.0f);
		REQUIRE(m1 == m2);
		REQUIRE(!(m1 == m3));
	}
}
