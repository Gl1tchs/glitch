#include <catch2/catch_test_macros.hpp>

#include "core/buffer.h"

constexpr uint8_t TEST_DATA[] = {
	0x0,
	0x2,
	0x4,
	0x8,
	0x16,
	0x32,
};
constexpr size_t TEST_DATA_SIZE = sizeof(TEST_DATA);
constexpr size_t TEST_DATA_COUNT = TEST_DATA_SIZE / sizeof(uint8_t);

TEST_CASE("Buffer", "[buffer]") {
	Buffer buffer(TEST_DATA, TEST_DATA_SIZE);

	REQUIRE((bool)buffer);

	REQUIRE(buffer.size == TEST_DATA_SIZE);

	uint8_t* data = buffer.as<uint8_t>();

	REQUIRE(data[0] == 0x0);
	REQUIRE(data[2] == 0x4);
	REQUIRE(data[4] == 0x16);
}

TEST_CASE("Buffer Array", "[buffer_array]") {
	BufferArray<uint8_t> buffer(TEST_DATA_COUNT);
	for (int i = 0; i < TEST_DATA_COUNT; i++) {
		buffer.add(TEST_DATA[i]);
	}

	SECTION("Buffer Array Allocation") {
		REQUIRE((bool)buffer);

		REQUIRE(buffer.get_count() == TEST_DATA_COUNT);
		REQUIRE(buffer.get_size() == TEST_DATA_SIZE);

		REQUIRE(buffer.at(0) == 0x0);
		REQUIRE(buffer.at(3) == 0x8);
		REQUIRE(buffer.at(5) == 0x32);
	}

	SECTION("Buffer Array Cleaning") {
		buffer.reset_index();

		REQUIRE(buffer.get_count() == 0);

		buffer.clear();

		REQUIRE(buffer.get_count() == 0);
	}

	SECTION("Buffer Array Deallocation") {
		buffer.release();

		REQUIRE(buffer.get_size() == 0);
		REQUIRE_FALSE((bool)buffer);
	}
}
