#pragma once

enum class LogLevel {
	TRACE = 0,
	INFO,
	WARNING,
	ERROR,
	FATAL,
};

std::string deserialize_log_level(LogLevel level);

class Logger {
public:
	static void log(LogLevel level, const std::string& fmt);

private:
	static std::string _get_colored_message(
			const std::string& message, LogLevel level);

private:
	static std::unordered_map<LogLevel, std::string> s_verbosity_colors;
};

#define GL_LOG_TRACE(...) Logger::log(LogLevel::TRACE, std::format(__VA_ARGS__))
#define GL_LOG_INFO(...) Logger::log(LogLevel::INFO, std::format(__VA_ARGS__))
#define GL_LOG_WARNING(...)                                                    \
	Logger::log(LogLevel::WARNING, std::format(__VA_ARGS__))
#define GL_LOG_ERROR(...) Logger::log(LogLevel::ERROR, std::format(__VA_ARGS__))
#define GL_LOG_FATAL(...) Logger::log(LogLevel::FATAL, std::format(__VA_ARGS__))