#include "glitch/core/config.h"

namespace gl {

enum class ConfigToken {
	TABLE,
	KEY,
	VALUE,
	COMMENT,
	END_OF_FILE,
};

struct ConfigTokenInfo {
	ConfigToken token;
	size_t begin;
	size_t end;
};

static Result<std::vector<ConfigTokenInfo>, ConfigParseError> _tokenize_config(
		std::string_view p_config) {
	enum class TokenizerState {
		KEY,
		VALUE,
	};

	std::vector<ConfigTokenInfo> tokens;

	TokenizerState state = TokenizerState::KEY; // we are always starting with a key

	// Begin and end locations of current token
	size_t idx = 0;
	size_t curr_begin = 0;
	size_t curr_end = 0;

	const size_t size = p_config.size();
	while (idx < size) {
		switch (p_config[idx]) {
			case ' ': {
				// this is the tricky part:
				// check for syntax errors by making sure there is no
				// spaces between variable or value declarations
				if (idx + 1 < size) {
					const char next_chr = p_config[idx + 1];
					if (next_chr != '=' || next_chr != '\n') {
						return make_err<std::vector<ConfigTokenInfo>>(
								ConfigParseError::SYNTAX_ERROR);
					}
				}

				break;
			}
			case '=': {
				// begin value declaration
				curr_end = idx;

				tokens.push_back({
						.token = ConfigToken::KEY,
						.begin = curr_begin,
						.end = curr_end,
				});

				// Update the state
				state = TokenizerState::VALUE;
				curr_begin = idx + 1; // TODO! check next idx

				break;
			}
			case '\n': {
				// end of value declaration
				curr_end = idx - 1;

				tokens.push_back({
						.token = ConfigToken::VALUE,
						.begin = curr_begin,
						.end = curr_end,
				});

				state = TokenizerState::KEY;
			}
			default: {
				// a key or a value
				switch (state) {
					case TokenizerState::KEY: {
						break;
					}
					case TokenizerState::VALUE: {
						break;
					}
				}
				break;
			}
		}

		idx++;
	}

	return tokens;
}

static Result<Config, ConfigParseError> _parse_config() {}

Result<Config, ConfigParseError> Config::from_string(const std::string& p_string) {
	Config config;

	return config;
}

Result<Config, ConfigParseError> Config::from_file(const fs::path& p_path) {}

Optional<ConfigValue> Config::get_value(const std::string& p_key) {
	const auto it = data.find(p_key);
	if (it == data.end()) {
		return std::nullopt;
	}

	return it->second;
}

void Config::set_value(const std::string& p_key, bool p_value) { data[p_key] = p_value; }

void Config::set_value(const std::string& p_key, const std::string& p_value) {
	data[p_key] = p_value;
}

void Config::set_value(const std::string& p_key, int p_value) { data[p_key] = p_value; }

void Config::set_value(const std::string& p_key, float p_value) { data[p_key] = p_value; }

void Config::set_value(const std::string& p_key, const glm::vec2& p_value) {
	data[p_key] = p_value;
}

void Config::set_value(const std::string& p_key, const glm::vec3& p_value) {
	data[p_key] = p_value;
}

void Config::set_value(const std::string& p_key, const glm::vec4& p_value) {
	data[p_key] = p_value;
}

} // namespace gl
