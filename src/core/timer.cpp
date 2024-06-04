#include "core/timer.h"

Timer::Timer() : last_frame_time(Clock::now()) {}

float Timer::get_elapsed_milliseconds() {
	const auto current_time = Clock::now();
	std::chrono::duration<float, std::chrono::milliseconds::period>
			elapsed_time = current_time - last_frame_time;
	return elapsed_time.count();
}

float Timer::get_elapsed_seconds() {
	const auto current_time = Clock::now();
	std::chrono::duration<float> elapsed_time = current_time - last_frame_time;
	return elapsed_time.count();
}

float Timer::get_delta_time() {
	const auto current_time = Clock::now();
	std::chrono::duration<float> delta_time = current_time - last_frame_time;
	last_frame_time = current_time;
	return delta_time.count();
}
