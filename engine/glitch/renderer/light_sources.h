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

GL_DEFINE_SERIALIZABLE(DirectionalLight, direction, color);

struct PointLight {
	glm::vec4 position;
	Color color;
	float linear;
	float quadratic;
	float _pad[2];
};

GL_DEFINE_SERIALIZABLE(PointLight, position, color, linear, quadratic);

template <typename T>
concept LightSource = IsAnyOf<T, DirectionalLight, PointLight>;

} //namespace gl