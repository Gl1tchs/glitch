/**
 * @file light_sources.h
 */

#pragma once

#include "glitch/core/core.h"
#include "glitch/core/templates/concepts.h"

#include <glgpu/glgpu.h>

namespace gl {

struct DirectionalLight {
	Vec4f direction;
	Color color;
};

GL_DEFINE_SERIALIZABLE(DirectionalLight, direction, color);

struct PointLight {
	Vec4f position;
	Color color;
	float linear;
	float quadratic;
	float _pad[2];
};

GL_DEFINE_SERIALIZABLE(PointLight, position, color, linear, quadratic);

template <typename T>
concept LightSource = IsAnyOf<T, DirectionalLight, PointLight>;

} //namespace gl