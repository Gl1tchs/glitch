#include "glitch/core/json.h"
#include "glitch/asset/asset_system.h"

#include <glgpu/result.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>

namespace gl {

Result<json, JSONLoadError> json_load(std::string_view path) {
	const auto abs_path = AssetSystem::get_absolute_path(path);
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

JSONLoadError json_save(std::string_view path, const json& j) {
	const auto abs_path = AssetSystem::get_absolute_path(path);
	if (!abs_path) {
		return JSONLoadError::INVALID_PATH;
	}

	std::ofstream f(*abs_path);
	if (!f.is_open()) {
		return JSONLoadError::FILE_OPEN_ERROR;
	}

	try {
		f << j.dump(2);

		return JSONLoadError::NONE;
	} catch (const std::runtime_error&) {
		return JSONLoadError::PARSING_ERROR;
	}
}

} //namespace gl
