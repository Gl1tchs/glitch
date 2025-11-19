/**
 * @file json.h
 *
 */

#pragma once

#include "glitch/core/templates/result.h"

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

namespace std {
template <typename T> void to_json(json& p_json, const std::optional<T>& p_opt) {
	p_json = p_opt ? json(*p_opt) : json(json::value_t::null);
}

template <typename T> void from_json(const json& p_json, std::optional<T>& p_opt) {
	p_opt = p_json.is_null() ? std::nullopt : std::optional<T>(p_json.get<T>());
}

} //namespace std

namespace gl {

enum class JSONLoadError {
	NONE,
	INVALID_PATH,
	FILE_OPEN_ERROR,
	PARSING_ERROR,
};

Result<json, JSONLoadError> json_load(std::string_view p_path);

JSONLoadError json_save(std::string_view p_path, const json& p_json);

} //namespace gl
