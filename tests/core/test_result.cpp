#include <doctest/doctest.h>

#include "glitch/core/templates/result.h"

gl::Result<float, bool> divide(int a, int b) {
	if (b == 0) {
		return gl::make_err<float>(false);
	}

	return (float)a / (float)b;
}

TEST_CASE("Result tests") {
	auto res = divide(6, 2);

	CHECK(res.has_value());
	CHECK((bool)res);

	CHECK(res.get_value() == 3.0f);
	CHECK(*res == 3.0f);

	res = divide(4, 0);

	CHECK(res.has_error());
	CHECK(res.get_error() == false);
}