#pragma once

namespace chrono = std::chrono;

class Timer final {
public:
	Timer();

	float get_elapsed_milliseconds();

	float get_elapsed_seconds();

	float get_delta_time();

private:
	chrono::time_point<chrono::high_resolution_clock> last_frame_time;
};
