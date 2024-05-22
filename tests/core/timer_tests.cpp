#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <thread>

#include "gl/core/timer.h"

using namespace std::chrono_literals;

TEST_CASE("Timer Elapsed Milliseconds", "[timer]") {
	Timer timer;

	std::this_thread::sleep_for(100ms);

	REQUIRE(timer.get_elapsed_milliseconds() == Catch::Approx(100).margin(5));
}

TEST_CASE("Timer Elapsed Seconds", "[timer]") {
	Timer timer;

	std::this_thread::sleep_for(1s);

	REQUIRE(timer.get_elapsed_seconds() == Catch::Approx(1).epsilon(0.01f));
}
