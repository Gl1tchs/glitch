#include "core/log.h"

std::string deserialize_log_level(LogLevel level) {
	switch (level) {
		case LogLevel::TRACE:
			return "TRACE";
		case LogLevel::INFO:
			return "INFO";
		case LogLevel::WARNING:
			return "WARNING";
		case LogLevel::ERROR:
			return "ERROR";
		case LogLevel::FATAL:
			return "FATAL";
		default:
			return "";
	}
}

inline static std::string get_timestamp() {
	auto now = std::chrono::system_clock::to_time_t(
			std::chrono::system_clock::now());

	std::tm tm_now{};
	std::stringstream ss;
#if EVE_PLATFORM_WINDOWS
	localtime_s(&tm_now, &now);
#else
	localtime_r(&now, &tm_now);
#endif

	ss << std::put_time(&tm_now, "%H:%M:%S");

	return ss.str();
}

std::unordered_map<LogLevel, std::string> Logger::s_verbosity_colors = {
	{ LogLevel::TRACE, "\x1B[1m" }, // None
	{ LogLevel::INFO, "\x1B[32m" }, // Green
	{ LogLevel::WARNING, "\x1B[93m" }, // Yellow
	{ LogLevel::ERROR, "\x1B[91m" }, // Light Red
	{ LogLevel::FATAL, "\x1B[31m" }, // Red
};

void Logger::log(LogLevel level, const std::string& fmt) {
	const std::string time_stamp = get_timestamp();

	const std::string message = std::format(
			"[{}] [{}]: {}", time_stamp, deserialize_log_level(level), fmt);

	const std::string colored_messages = _get_colored_message(message, level);

	// Output to stdout
	std::cout << colored_messages << "\x1B[0m\n";
}

std::string Logger::_get_colored_message(
		const std::string& message, LogLevel level) {
	const auto color_it = s_verbosity_colors.find(level);
	if (color_it != s_verbosity_colors.end()) {
		return color_it->second + message;
	}

	return message; // No color for the default case
}
