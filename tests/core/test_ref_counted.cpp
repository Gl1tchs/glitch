#include <doctest/doctest.h>

#include "glitch/core/ref_counted.h"

using namespace gl;

TEST_CASE("Test RefCounted") {
	RefCounted<uint32_t> r1(1);

	CHECK(r1.get_ref_count() == 1);

	// Copy - ref count must be two
	auto r2 = r1;

	CHECK(r1.get_ref_count() == r2.get_ref_count());
	CHECK(r1.get_ref_count() == 2);

	// Values must point into the same memory
	r1.get_value() = 54;

	CHECK(r2.get_value() == 54);

	// Move reference
	auto r3 = std::move(r2);

	// Check reference counts
	CHECK(r2.get_ref_count() == 0);
	CHECK(r3.get_ref_count() == 2);

	// Check value
	CHECK(r3.get_value() == 54);
}