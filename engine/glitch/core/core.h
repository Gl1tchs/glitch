#pragma once

#include "glitch/core/debug/assert.h"
#include "glitch/core/memory/memory.h"
#include "glitch/core/result.h"

#include <json/json.hpp>

using json = nlohmann::json;

namespace glm {
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(vec2, x, y);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(vec3, x, y, z);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(vec4, x, y, z, w);
} //namespace glm
