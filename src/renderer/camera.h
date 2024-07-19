#pragma once

#include "core/transform.h"

struct GL_API Camera {
	float aspect_ratio = 1.0f;
	float near_clip = -1.0f;
	float far_clip = 1.0f;

	virtual glm::mat4 get_view_matrix(Transform& p_transform) const = 0;
	virtual glm::mat4 get_projection_matrix() const = 0;
};

struct GL_API OrthographicCamera : Camera {
	float zoom_level = 1.0f;

	OrthographicCamera();
	~OrthographicCamera() = default;

	glm::mat4 get_view_matrix(Transform& p_transform) const override;
	glm::mat4 get_projection_matrix() const override;
};

struct GL_API PerspectiveCamera : Camera {
	float fov = 45.0f;

	PerspectiveCamera();
	~PerspectiveCamera() = default;

	glm::mat4 get_view_matrix(Transform& p_transform) const override;
	glm::mat4 get_projection_matrix() const override;
};
