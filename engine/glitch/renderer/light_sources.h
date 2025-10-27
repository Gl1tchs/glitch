/**
 * @file light_sources.h
 */

#pragma once

#include "glitch/core/color.h"

namespace gl {

struct DirectionalLight {
	glm::vec4 direction;
	Color color;
};

struct PointLight {
	glm::vec4 position;
	Color color;
	float linear;
	float quadratic;
	float _pad[2];
};

template <typename T>
concept LightSource = IsAnyOf<T, DirectionalLight, PointLight>;

} //namespace gl