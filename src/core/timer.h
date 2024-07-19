#pragma once

class GL_API Timer {
public:
	using Clock = std::chrono::high_resolution_clock;

	Timer();

	float get_elapsed_milliseconds();

	float get_elapsed_seconds();

	float get_delta_time();

private:
	std::chrono::time_point<Clock> last_frame_time;
};
