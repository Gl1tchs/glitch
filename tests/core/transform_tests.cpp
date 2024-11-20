#include <doctest.h>

#include "glitch/core/transform.h"

TEST_CASE("Transform initialization") {
	Transform t;

	CHECK(t.get_position() == VEC3_ZERO);
	CHECK(t.get_scale() == VEC3_ONE);
	CHECK(t.get_rotation() == VEC3_ZERO);
}

TEST_CASE("Translate transform") {
	Transform t;
	glm::vec3 translation(1.0f, 2.0f, 3.0f);
	t.translate(translation);

	CHECK(t.get_position() == translation);
}

TEST_CASE("Rotate transform") {
	Transform t;
	t.rotate(90.0f, VEC3_UP);

	CHECK(t.get_rotation() == glm::vec3{ 0.0f, 90.0f, 0.0f });
}

TEST_CASE("Transform directions") {
	Transform t;

	CHECK(t.get_forward() == VEC3_FORWARD);
	CHECK(t.get_right() == VEC3_RIGHT);
	CHECK(t.get_up() == VEC3_UP);
}

TEST_CASE("Transform matrix") {
	Transform t;
	glm::mat4 matrix = t.get_transform_matrix();

	CHECK(matrix == glm::mat4(1.0f));
}

TEST_CASE("Child/Parent relations") {
	Transform t1 = {
		.local_position = { 0, 1, 0 },
	};

	Transform t2 = {
		.parent = &t1,
		.local_position = { 0, -1, 0 },
	};

	CHECK(t2.get_position() == VEC3_ZERO);
}
