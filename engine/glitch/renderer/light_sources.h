/**
 * @file light_sources.h
 */

#pragma once

#include "glitch/core/color.h"

namespace gl {

struct alignas(16) DirectionalLight {
	glm::vec3 direction;
	float _pad0;
	glm::vec3 color;
	float _pad1;
};

struct alignas(16) PointLight {
	glm::vec3 position;
	float _pad0;
	glm::vec3 color;
	float _pad1;
	float linear;
	float quadratic;
	float _pad2[2];
};

struct alignas(16) Spotlight {
	glm::vec3 position;
	float _pad0;
	glm::vec3 direction;
	float _pad1;
	glm::vec3 color;
	float cut_off;
	float _pad2[3];
};

template <typename T>
concept LightSource = IsAnyOf<T, DirectionalLight, PointLight, Spotlight>;

} //namespace gl