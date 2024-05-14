#pragma once

struct Color {
	float r = 0.0f;
	float g = 0.0f;
	float b = 0.0f;
	float a = 1.0f;

	constexpr Color() = default;

	constexpr Color(float value) : r(value), g(value), b(value), a(1.0f) {}

	constexpr Color(float red, float green, float blue, float alpha) :
			r(red), g(green), b(blue), a(alpha) {}
};

constexpr Color COLOR_BLACK(0.0f, 0.0f, 0.0f, 1.0f);
constexpr Color COLOR_WHITE(1.0f, 1.0f, 1.0f, 1.0f);
constexpr Color COLOR_RED(1.0f, 0.0f, 0.0f, 1.0f);
constexpr Color COLOR_GREEN(0.0f, 1.0f, 0.0f, 1.0f);
constexpr Color COLOR_BLUE(0.0f, 0.0f, 1.0f, 1.0f);
constexpr Color COLOR_YELLOW(1.0f, 1.0f, 0.0f, 1.0f);
constexpr Color COLOR_CYAN(0.0f, 1.0f, 1.0f, 1.0f);
constexpr Color COLOR_MAGENTA(1.0f, 0.0f, 1.0f, 1.0f);
constexpr Color COLOR_GRAY(0.5f, 0.5f, 0.5f, 1.0f);
constexpr Color COLOR_ORANGE(1.0f, 0.5f, 0.0f, 1.0f);
constexpr Color COLOR_TRANSPARENT(0.0f, 0.0f, 0.0f, 0.0f);
