/**
 * @file json.h
 *
 */

#pragma once

#include "glitch/core/result.h"

#include <json/json.hpp>

using json = nlohmann::json;

// just to make things look better
#define GL_DEFINE_SERIALIZABLE(Type, ...) NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Type, __VA_ARGS__)
#define GL_SERIALIZE_ENUM(Type, ...) NLOHMANN_JSON_SERIALIZE_ENUM(Type, __VA_ARGS__)

namespace glm {
GL_DEFINE_SERIALIZABLE(vec2, x, y);
GL_DEFINE_SERIALIZABLE(vec3, x, y, z);
GL_DEFINE_SERIALIZABLE(vec4, x, y, z, w);
} //namespace glm

namespace gl {

enum class JSONLoadError {
	NONE,
	INVALID_PATH,
	FILE_OPEN_ERROR,
	PARSING_ERROR,
};

Result<json, JSONLoadError> load_json(std::string_view p_path);

JSONLoadError save_json(std::string_view p_path, const json& p_json);

} //namespace gl
