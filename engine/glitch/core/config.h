/**
 * @file config.h
 *
 */

#pragma once

#include "glitch/core/templates/concepts.h"

namespace gl {

/**
 * title = "Glitch Editor"
 * size = 500,500
 * \r\n as delimeter
 *
 */

template <typename T>
concept ConfigValueType =
		IsAnyOf<T, bool, int, float, std::string>;

using ConfigValue = std::variant<bool, int, float, std::string>;

enum class ConfigParseError {
  FILE_NOT_FOUND,
  SYNTAX_ERROR,
};

/**
 * Class representing an flat ini configuration file
 *
 */
class Config {
public:
	Config() = default;
	~Config() = default;

	static Result<Config, ConfigParseError> from_string(const std::string& p_string);
	static Result<Config, ConfigParseError> from_file(const fs::path& p_path);

	Optional<ConfigValue> get_value(const std::string& p_key);

	template <typename T> Optional<T> get_value(const std::string& p_key);

	void set_value(const std::string& p_key, bool p_value);
	void set_value(const std::string& p_key, const std::string& p_value);
	void set_value(const std::string& p_key, int p_value);
	void set_value(const std::string& p_key, float p_value);
	void set_value(const std::string& p_key, const glm::vec2& p_value);
	void set_value(const std::string& p_key, const glm::vec3& p_value);
	void set_value(const std::string& p_key, const glm::vec4& p_value);

private:
	std::unordered_map<std::string, ConfigValue> data;
};

template <typename T> inline Optional<T> Config::get_value(const std::string& p_key) {
	Optional<ConfigValue> value = get_value(p_key);
	if (!value) {
		return std::nullopt;
	}

	if (const T* p_value = std::get_if<T>(*value)) {
		return *p_value;
	}

	return std::nullopt;
}

} //namespace gl
