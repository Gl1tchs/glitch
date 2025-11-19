#include "glitch/core/json.h"

#include "glitch/asset/asset_system.h"

namespace gl {

Result<json, JSONLoadError> load_json(std::string_view p_path) {
	const auto abs_path = AssetSystem::get_absolute_path(p_path);
	if (!abs_path) {
		return make_err<json>(JSONLoadError::INVALID_PATH);
	}

	std::ifstream f(*abs_path);
	if (!f.is_open()) {
		return make_err<json>(JSONLoadError::FILE_OPEN_ERROR);
	}

	try {
		json j;
		f >> j;

		return j;
	} catch (const std::runtime_error&) {
		return make_err<json>(JSONLoadError::PARSING_ERROR);
	}
}

JSONLoadError save_json(std::string_view p_path, const json& p_json) {
	const auto abs_path = AssetSystem::get_absolute_path(p_path);
	if (!abs_path) {
		return JSONLoadError::INVALID_PATH;
	}

	std::ofstream f(*abs_path);
	if (!f.is_open()) {
		return JSONLoadError::FILE_OPEN_ERROR;
	}

	try {
		f << p_json;

		return JSONLoadError::NONE;
	} catch (const std::runtime_error&) {
		return JSONLoadError::PARSING_ERROR;
	}
}

} //namespace gl
