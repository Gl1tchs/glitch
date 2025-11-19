#include "glitch/core/debug/log.h"

namespace gl {

constexpr const char* LOG_LEVEL_TO_STR[] = {
	[LOG_LEVEL_TRACE] = "TRACE",
	[LOG_LEVEL_INFO] = "INFO",
	[LOG_LEVEL_WARNING] = "WARNING",
	[LOG_LEVEL_ERROR] = "ERROR",
	[LOG_LEVEL_FATAL] = "FATAL",
};

constexpr const char* VERBOSITY_TO_COLOR[] = {
	[LOG_LEVEL_TRACE] = "\x1B[1m", // None
	[LOG_LEVEL_INFO] = "\x1B[32m", // Green
	[LOG_LEVEL_WARNING] = "\x1B[93m", // Yellow
	[LOG_LEVEL_ERROR] = "\x1B[91m", // Light Red
	[LOG_LEVEL_FATAL] = "\x1B[31m", // Red
};

static std::string _get_timestamp() {
	const auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

	std::tm tm_now{};
	std::stringstream ss;
#if GL_PLATFORM_WINDOWS
	localtime_s(&tm_now, &now);
#else
	localtime_r(&now, &tm_now);
#endif

	ss << std::put_time(&tm_now, "%H:%M:%S");

	return ss.str();
}

void Logger::log(LogLevel p_level, const std::string& p_fmt) {
	// Output to stdout
	std::cout << VERBOSITY_TO_COLOR[p_level]
			  << std::format("[{}] [{}] {}", _get_timestamp(), LOG_LEVEL_TO_STR[p_level], p_fmt)
			  << "\x1B[0m\n";
}

} //namespace gl
