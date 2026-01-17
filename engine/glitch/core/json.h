/**
 * @file json.h
 *
 */

#pragma once

#include <json/json.hpp>

using json = nlohmann::json;

// just to make things look better
#define GL_DEFINE_SERIALIZABLE(Type, ...) NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Type, __VA_ARGS__)
#define GL_SERIALIZE_ENUM(Type, ...) NLOHMANN_JSON_SERIALIZE_ENUM(Type, __VA_ARGS__)

#include <glgpu/glgpu.h>

namespace std {
template <typename T> void to_json(json& j, const std::optional<T>& opt) {
	j = opt ? json(*opt) : json(json::value_t::null);
}

template <typename T> void from_json(const json& j, std::optional<T>& opt) {
	opt = j.is_null() ? std::nullopt : std::optional<T>(j.get<T>());
}

} //namespace std

namespace gl {

GL_DEFINE_SERIALIZABLE(Vec2f, x, y);
GL_DEFINE_SERIALIZABLE(Vec3f, x, y, z);
GL_DEFINE_SERIALIZABLE(Vec4f, x, y, z, w);
GL_DEFINE_SERIALIZABLE(Color, r, g, b, a);

enum class JSONLoadError {
	NONE,
	INVALID_PATH,
	FILE_OPEN_ERROR,
	PARSING_ERROR,
};

Result<json, JSONLoadError> json_load(std::string_view path);

JSONLoadError json_save(std::string_view path, const json& j);

} //namespace gl
