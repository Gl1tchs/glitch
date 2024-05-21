#pragma once

#include "core/transform.h"

struct Camera {
	Transform transform;

	float aspect_ratio = 1.0f;
	float near_clip = -1.0f;
	float far_clip = 1.0f;

	virtual glm::mat4 get_view_matrix() const = 0;
	virtual glm::mat4 get_projection_matrix() const = 0;
};

struct OrthographicCamera : Camera {
	float aspect_ratio = 1.0f;
	float zoom_level = 1.0f;
	float near_clip = -1.0f;
	float far_clip = 1.0f;

	glm::mat4 get_view_matrix() const override;
	glm::mat4 get_projection_matrix() const override;
};

struct PerspectiveCamera : Camera {
	float aspect_ratio = 1.0f;
	float fov = 45.0f;
	float near_clip = 0.1f;
	float far_clip = 300.0f;

	~PerspectiveCamera() = default;

	glm::mat4 get_view_matrix() const override;
	glm::mat4 get_projection_matrix() const override;
};
