#include <catch2/catch_test_macros.hpp>

#include "core/templates/vector.h"

TEST_CASE("Vec2 Dot Product", "[vec2]") {
	Vec2f v1(9.0f, 5.0f);
	Vec2f v2(3.0f, 4.0f);
	float res = v1.dot(v2);
	REQUIRE(res == 47.0f);
}

TEST_CASE("Vec2 Arithmetic Operators", "[vec2]") {
	SECTION("Addition") {
		Vec2f v1(1.0f, 2.0f);
		Vec2f v2(3.0f, 4.0f);
		Vec2f v3 = v1 + v2;
		REQUIRE(v3.x == 4.0f);
		REQUIRE(v3.y == 6.0f);
	}

	SECTION("Subtraction") {
		Vec2f v1(5.0f, 6.0f);
		Vec2f v2(3.0f, 4.0f);
		Vec2f v3 = v1 - v2;
		REQUIRE(v3.x == 2.0f);
		REQUIRE(v3.y == 2.0f);
	}

	SECTION("Multiplication by scalar") {
		Vec2f v1(2.0f, 3.0f);
		Vec2f v2 = v1 * 2.0f;
		REQUIRE(v2.x == 4.0f);
		REQUIRE(v2.y == 6.0f);
	}

	SECTION("Division by scalar") {
		Vec2f v1(6.0f, 8.0f);
		Vec2f v2 = v1 / 2.0f;
		REQUIRE(v2.x == 3.0f);
		REQUIRE(v2.y == 4.0f);
	}
}

TEST_CASE("Vec2 Compound Assignment Operators", "[vec2]") {
	SECTION("Addition Assignment") {
		Vec2f v1(1.0f, 2.0f);
		Vec2f v2(3.0f, 4.0f);
		v1 += v2;
		REQUIRE(v1.x == 4.0f);
		REQUIRE(v1.y == 6.0f);
	}

	SECTION("Subtraction Assignment") {
		Vec2f v1(5.0f, 6.0f);
		Vec2f v2(3.0f, 4.0f);
		v1 -= v2;
		REQUIRE(v1.x == 2.0f);
		REQUIRE(v1.y == 2.0f);
	}

	SECTION("Multiplication Assignment") {
		Vec2f v1(2.0f, 3.0f);
		v1 *= 2.0f;
		REQUIRE(v1.x == 4.0f);
		REQUIRE(v1.y == 6.0f);
	}

	SECTION("Division Assignment") {
		Vec2f v1(6.0f, 8.0f);
		v1 /= 2.0f;
		REQUIRE(v1.x == 3.0f);
		REQUIRE(v1.y == 4.0f);
	}
}

TEST_CASE("Vec2 Comparison Operators", "[vec2]") {
	SECTION("Equality") {
		Vec2f v1(1.0f, 2.0f);
		Vec2f v2(1.0f, 2.0f);
		Vec2f v3(3.0f, 4.0f);
		REQUIRE(v1 == v2);
		REQUIRE(!(v1 == v3));
	}
}

TEST_CASE("Vec3 Dot Product", "[vec3]") {
	Vec3f v1(9.0f, 5.0f, 1.0f);
	Vec3f v2(3.0f, 4.0f, 3.0f);
	float res = v1.dot(v2);
	REQUIRE(res == 50.0f);
}

TEST_CASE("Vec3 Arithmetic Operators", "[vec3]") {
	SECTION("Addition") {
		Vec3f v1(1.0f, 2.0f, 3.0f);
		Vec3f v2(4.0f, 5.0f, 6.0f);
		Vec3f v3 = v1 + v2;
		REQUIRE(v3.x == 5.0f);
		REQUIRE(v3.y == 7.0f);
		REQUIRE(v3.z == 9.0f);
	}

	SECTION("Subtraction") {
		Vec3f v1(6.0f, 7.0f, 8.0f);
		Vec3f v2(3.0f, 4.0f, 5.0f);
		Vec3f v3 = v1 - v2;
		REQUIRE(v3.x == 3.0f);
		REQUIRE(v3.y == 3.0f);
		REQUIRE(v3.z == 3.0f);
	}

	SECTION("Multiplication by scalar") {
		Vec3f v1(4.9f, 9.0f, 7.0f);
		Vec3f v2 = v1 * 2.0f;
		REQUIRE(v2.x == 9.8f);
		REQUIRE(v2.y == 18.0f);
		REQUIRE(v2.z == 14.0f);
	}

	SECTION("Division by scalar") {
		Vec3f v1(6.0f, 8.0f, 4.0f);
		Vec3f v2 = v1 / 2.0f;
		REQUIRE(v2.x == 3.0f);
		REQUIRE(v2.y == 4.0f);
		REQUIRE(v2.z == 2.0f);
	}
}

TEST_CASE("Vec3 Compound Assignment Operators", "[vec3]") {
	SECTION("Addition Assignment") {
		Vec3f v1(1.0f, 2.0f, 3.0f);
		Vec3f v2(4.0f, 5.0f, 6.0f);
		v1 += v2;
		REQUIRE(v1.x == 5.0f);
		REQUIRE(v1.y == 7.0f);
		REQUIRE(v1.z == 9.0f);
	}

	SECTION("Subtraction Assignment") {
		Vec3f v1(6.0f, 7.0f, 8.0f);
		Vec3f v2(3.0f, 4.0f, 5.0f);
		v1 -= v2;
		REQUIRE(v1.x == 3.0f);
		REQUIRE(v1.y == 3.0f);
		REQUIRE(v1.z == 3.0f);
	}

	SECTION("Multiplication Assignment") {
		Vec3f v1(2.0f, 3.0f, 4.0f);
		v1 *= 2.0f;
		REQUIRE(v1.x == 4.0f);
		REQUIRE(v1.y == 6.0f);
		REQUIRE(v1.z == 8.0f);
	}

	SECTION("Division Assignment") {
		Vec3f v1(6.0f, 8.0f, 10.0f);
		v1 /= 2.0f;
		REQUIRE(v1.x == 3.0f);
		REQUIRE(v1.y == 4.0f);
		REQUIRE(v1.z == 5.0f);
	}
}

TEST_CASE("Vec3 Comparison Operators", "[vec3]") {
	SECTION("Equality") {
		Vec3f v1(1.0f, 2.0f, 3.0f);
		Vec3f v2(1.0f, 2.0f, 3.0f);
		Vec3f v3(3.0f, 4.0f, 5.0f);
		REQUIRE(v1 == v2);
		REQUIRE(v1 != v3);
	}
}
