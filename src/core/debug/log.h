#pragma once

#include "core/defines.h"

enum LogLevel {
	LOG_LEVEL_TRACE = 0,
	LOG_LEVEL_INFO,
	LOG_LEVEL_WARNING,
	LOG_LEVEL_ERROR,
	LOG_LEVEL_FATAL,
};

std::string deserialize_log_level(LogLevel p_level);

class GL_API Logger {
public:
	static void log(LogLevel p_level, const std::string& p_fmt);

private:
	static std::string _get_colored_message(
			const std::string& p_message, LogLevel p_level);

private:
	static std::unordered_map<LogLevel, std::string> s_verbosity_colors;
};

#define GL_LOG_TRACE(...) Logger::log(LOG_LEVEL_TRACE, std::format(__VA_ARGS__))
#define GL_LOG_INFO(...) Logger::log(LOG_LEVEL_INFO, std::format(__VA_ARGS__))
#define GL_LOG_WARNING(...)                                                    \
	Logger::log(LOG_LEVEL_WARNING, std::format(__VA_ARGS__))
#define GL_LOG_ERROR(...) Logger::log(LOG_LEVEL_ERROR, std::format(__VA_ARGS__))
#define GL_LOG_FATAL(...) Logger::log(LOG_LEVEL_FATAL, std::format(__VA_ARGS__))
