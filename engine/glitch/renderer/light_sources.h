/**
 * @file light_sources.h
 */

#pragma once

#include "glitch/core/color.h"
#include "glitch/core/templates/concepts.h"

namespace gl {

struct DirectionalLight {
	glm::vec4 direction;
	Color color;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DirectionalLight, direction, color);

struct PointLight {
	glm::vec4 position;
	Color color;
	float linear;
	float quadratic;
	float _pad[2];
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PointLight, position, color, linear, quadratic);

template <typename T>
concept LightSource = IsAnyOf<T, DirectionalLight, PointLight>;

} //namespace gl