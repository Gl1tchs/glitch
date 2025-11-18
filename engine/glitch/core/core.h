#pragma once

#include "glitch/core/debug/assert.h"
#include "glitch/core/memory/memory.h"
#include "glitch/core/result.h"

#include <json/json.hpp>

using json = nlohmann::json;

// just to make things look better
#define GL_DEFINE_SERIALIZABLE(Type, ...) NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Type, __VA_ARGS__)

namespace glm {
GL_DEFINE_SERIALIZABLE(vec2, x, y);
GL_DEFINE_SERIALIZABLE(vec3, x, y, z);
GL_DEFINE_SERIALIZABLE(vec4, x, y, z, w);
} //namespace glm
