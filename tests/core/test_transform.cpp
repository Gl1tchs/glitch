#include <catch2/catch_test_macros.hpp>

#include "glitch/core/transform.h"

using namespace gl;

TEST_CASE("Transform initialization") {
	Transform t;

	REQUIRE(t.local_position == Vec3f::zero());
	REQUIRE(t.local_scale == Vec3f::one());
	REQUIRE(t.local_rotation == Vec3f::zero());
}

TEST_CASE("Translate transform") {
	Transform t;
	Vec3f translation(1.0f, 2.0f, 3.0f);
	t.translate(translation);

	REQUIRE(t.local_position == translation);
}

TEST_CASE("Rotate transform") {
	Transform t;
	t.rotate(90.0f, Vec3f::up());

	REQUIRE(t.local_rotation == Vec3f{ 0.0f, 90.0f, 0.0f });
}

TEST_CASE("Transform directions") {
	Transform t;

	REQUIRE(t.get_forward() == Vec3f::forward());
	REQUIRE(t.get_right() == Vec3f::right());
	REQUIRE(t.get_up() == Vec3f::up());
}

TEST_CASE("Transform matrix") {
	Transform t;
	Mat4 matrix = t.to_mat4();

	REQUIRE(matrix == Mat4(1.0f));
}
