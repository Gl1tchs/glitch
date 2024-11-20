#include "glitch/core/color.h"

#include <doctest.h>

TEST_CASE("Color Default Constructor") {
	Color color;
	CHECK(color == Color(0.0f, 0.0f, 0.0f, 1.0f)); // Black with full opacity
}

TEST_CASE("Color Grayscale Constructor") {
	Color color(0.5f);
	CHECK(color ==
			Color(0.5f, 0.5f, 0.5f, 1.0f)); // All components should be equal
}

TEST_CASE("Color from uint32_t") {
	Color color1(0xff00ffff); // RGBA: 255, 0, 255, 255
	CHECK(color1 == Color(1.0f, 0.0f, 1.0f, 1.0f));

	Color color2(static_cast<uint32_t>(0x00000000)); // RGBA: 0, 0, 0, 0
	CHECK(color2 == Color(0.0f, 0.0f, 0.0f, 0.0f));

	Color color3(0xffffffff); // RGBA: 255, 255, 255, 255
	CHECK(color3 == Color(1.0f, 1.0f, 1.0f, 1.0f));
}

TEST_CASE("Color::as_uint Tests") {
	Color color1(1.0f, 0.0f, 0.0f, 1.0f); // RGBA: 255, 0, 0, 255
	CHECK(color1.as_uint() == 0xff0000ff);

	Color color2(0.5f, 0.25f, 0.75f, 1.0f); // RGBA: 128, 64, 192, 255
	CHECK(color2.as_uint() == 0x8040c0ff);

	Color color3(0.0f, 1.0f, 0.0f, 0.0f); // RGBA: 0, 255, 0, 0
	CHECK(color3.as_uint() == 0x00ff0000);
}

TEST_CASE("Color Round-Trip Conversion") {
	uint32_t packed_color = 0x11223344; // Arbitrary color
	Color color(packed_color);
	CHECK(color.as_uint() == packed_color);
}

TEST_CASE("Color Boundary Values") {
	Color black(0.0f, 0.0f, 0.0f, 1.0f);
	CHECK(black.as_uint() == 0x000000ff); // RGBA: 0, 0, 0, 255

	Color white(1.0f, 1.0f, 1.0f, 1.0f);
	CHECK(white.as_uint() == 0xffffffff); // RGBA: 255, 255, 255, 255

	Color red(1.0f, 0.0f, 0.0f, 1.0f);
	CHECK(red.as_uint() == 0xff0000ff); // RGBA: 255, 0, 0, 255

	Color green(0.0f, 1.0f, 0.0f, 1.0f);
	CHECK(green.as_uint() == 0x00ff00ff); // RGBA: 0, 255, 0, 255

	Color blue(0.0f, 0.0f, 1.0f, 1.0f);
	CHECK(blue.as_uint() == 0x0000ffff); // RGBA: 0, 0, 255, 255
}
