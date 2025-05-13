#include <doctest/doctest.h>

#include "glitch/core/math/quat.h"
#include "glitch/core/math/vector.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_PI_2
#define M_PI_2 (M_PI / 2.0)
#endif

TEST_CASE("Quaternion Default Constructor") {
	Quat q;
	CHECK(q.w == 1.0f);
	CHECK(q.x == 0.0f);
	CHECK(q.y == 0.0f);
	CHECK(q.z == 0.0f);
}

TEST_CASE("Quaternion Axis-Angle Constructor") {
	Vec3f axis(0.0f, 1.0f, 0.0f); // y-axis
	float angle = M_PI; // 180 degrees
	Quat q = Quat::from_axis_angle(axis, angle);

	// Rotating (1, 0, 0) around Y by 180° should result in (-1, 0, 0)
	Vec3f v(1.0f, 0.0f, 0.0f);
	Vec3f result = q * v;

	CHECK(doctest::Approx(result.x) == -1.0f);
	CHECK(doctest::Approx(result.y) == 0.0f);
	CHECK(doctest::Approx(result.z) == 0.0f);
}

TEST_CASE("Quaternion Euler Conversion") {
	// Rotation: 90° around Y axis
	Vec3f euler(0.0f, M_PI_2, 0.0f);
	Quat q = Quat::from_euler(euler);

	// Rotating (1, 0, 0) should result in (0, 0, -1)
	Vec3f v(1.0f, 0.0f, 0.0f);
	Vec3f result = q * v;

	CHECK(doctest::Approx(result.x).epsilon(0.01) == 0.0f);
	CHECK(doctest::Approx(result.y).epsilon(0.01) == 0.0f);
	CHECK(doctest::Approx(result.z).epsilon(0.01) == -1.0f);
}

TEST_CASE("Quaternion Normalize") {
	Quat q(0.0f, 2.0f, 0.0f, 0.0f);
	Quat n = q.normalize();
	float len = n.norm();

	CHECK(doctest::Approx(len).epsilon(0.001f) == 1.0f);
}

TEST_CASE("Quaternion Conjugate") {
	Quat q(1.0f, 2.0f, 3.0f, 4.0f);
	Quat c = q.conjugate();

	CHECK(c.w == 1.0f);
	CHECK(c.x == -2.0f);
	CHECK(c.y == -3.0f);
	CHECK(c.z == -4.0f);
}

TEST_CASE("Quaternion Multiplication (Rotation)") {
	// 180° around Z axis
	Quat q = Quat::from_axis_angle(Vec3f(0, 0, 1), M_PI);

	Vec3f v(1, 0, 0);
	Vec3f result = q * v;

	CHECK(doctest::Approx(result.x).epsilon(0.001f) == -1.0f);
	CHECK(doctest::Approx(result.y).epsilon(0.001f) == 0.0f);
	CHECK(doctest::Approx(result.z).epsilon(0.001f) == 0.0f);
}

TEST_CASE("Quaternion Dot Product") {
	Quat q1(1, 2, 3, 4);
	Quat q2(5, 6, 7, 8);

	float dot = q1.dot(q2);
	CHECK(dot == 1 * 5 + 2 * 6 + 3 * 7 + 4 * 8); // 70
}
