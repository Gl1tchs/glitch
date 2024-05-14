#include "core/timer.h"

Timer::Timer() : last_frame_time(chrono::high_resolution_clock::now()) {}

float Timer::get_elapsed_milliseconds() {
	auto current_time = chrono::high_resolution_clock::now();
	chrono::duration<float, chrono::milliseconds::period> elapsed_time =
			current_time - last_frame_time;
	return elapsed_time.count();
}

float Timer::get_elapsed_seconds() {
	auto current_time = chrono::high_resolution_clock::now();
	chrono::duration<float> elapsed_time = current_time - last_frame_time;
	return elapsed_time.count();
}

float Timer::get_delta_time() {
	auto current_time = chrono::high_resolution_clock::now();
	chrono::duration<float> delta_time = current_time - last_frame_time;
	last_frame_time = current_time;
	return delta_time.count();
}
