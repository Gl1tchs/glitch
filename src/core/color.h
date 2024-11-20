/**
 * @file color.h
 */

#pragma once

/**
 * Color representing Red,Green,Blue,Alpha values in 32-bit floats.
 */
struct Color {
	float r = 0.0f;
	float g = 0.0f;
	float b = 0.0f;
	float a = 1.0f;

	/**
	 * Gray-Scale constructor, fills every component
	 * except `alpha` to `p_value`
	 */
	explicit constexpr Color(float p_value = 0.0f) :
			r(p_value), g(p_value), b(p_value) {}

	constexpr Color(
			float p_red, float p_green, float p_blue, float p_alpha = 1.0f) :
			r(p_red), g(p_green), b(p_blue), a(p_alpha) {}

	/**
	 * Constructs `Color` object from RRGGBBAA packed unsigned integer
	 */
	constexpr Color(uint32_t p_value) :
			r(((p_value >> 24) & 0xFF) / 255.0f),
			g(((p_value >> 16) & 0xFF) / 255.0f),
			b(((p_value >> 8) & 0xFF) / 255.0f),
			a((p_value & 0xFF) / 255.0f) {}

	const float* get_ptr() const { return &r; }

	/**
	 * Get RRGGBBAA packed uint32_t representation of the `Color` object.
	 */
	constexpr uint32_t as_uint() const {
		constexpr auto to_uint8 = [](float p_value) {
			const float result = std::floor(p_value * 256.0f);
			return result > 255.0f ? 255 : static_cast<uint8_t>(result);
		};

		uint8_t red = to_uint8(r);
		uint8_t green = to_uint8(g);
		uint8_t blue = to_uint8(b);
		uint8_t alpha = to_uint8(a);

		return (red << 24) | (green << 16) | (blue << 8) | alpha;
	}

	constexpr bool operator==(const Color& p_other) const {
		return r == p_other.r && g == p_other.g && b == p_other.b &&
				a == p_other.a;
	}
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
