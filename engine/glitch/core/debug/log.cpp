#include "glitch/core/debug/log.h"

namespace gl {

const char* LOG_LEVEL_TO_STR[] = {
	[LOG_LEVEL_TRACE] = "TRACE",
	[LOG_LEVEL_INFO] = "INFO",
	[LOG_LEVEL_WARNING] = "WARNING",
	[LOG_LEVEL_ERROR] = "ERROR",
	[LOG_LEVEL_FATAL] = "FATAL",
};

inline static std::string get_timestamp() {
	const auto now = std::chrono::system_clock::to_time_t(
			std::chrono::system_clock::now());

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

std::unordered_map<LogLevel, std::string> Logger::s_verbosity_colors = {
	{ LOG_LEVEL_TRACE, "\x1B[1m" }, // None
	{ LOG_LEVEL_INFO, "\x1B[32m" }, // Green
	{ LOG_LEVEL_WARNING, "\x1B[93m" }, // Yellow
	{ LOG_LEVEL_ERROR, "\x1B[91m" }, // Light Red
	{ LOG_LEVEL_FATAL, "\x1B[31m" }, // Red
};

void Logger::log(LogLevel p_level, const std::string& p_fmt) {
	const std::string time_stamp = get_timestamp();

	const std::string message = std::format(
			"[{}] [{}]: {}", time_stamp, LOG_LEVEL_TO_STR[p_level], p_fmt);

	const std::string colored_messages = _get_colored_message(message, p_level);

	// Output to stdout
	std::cout << colored_messages << "\x1B[0m\n";
}

std::string Logger::_get_colored_message(
		const std::string& p_message, LogLevel p_level) {
	const auto color_it = s_verbosity_colors.find(p_level);
	if (color_it != s_verbosity_colors.end()) {
		return color_it->second + p_message;
	}

	return p_message; // No color for the default case
}

} //namespace gl