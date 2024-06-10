#pragma once

#include "renderer/node.h"

struct CameraNode : public Node {
	GL_IMPL_NODE(NodeType::CAMERA)

	float aspect_ratio = 1.0f;
	float near_clip = -1.0f;
	float far_clip = 1.0f;

	virtual glm::mat4 get_view_matrix() const = 0;
	virtual glm::mat4 get_projection_matrix() const = 0;
};

struct OrthographicCameraNode : CameraNode {
	float zoom_level = 1.0f;

	OrthographicCameraNode();
	~OrthographicCameraNode() = default;

	glm::mat4 get_view_matrix() const override;
	glm::mat4 get_projection_matrix() const override;
};

struct PerspectiveCameraNode : CameraNode {
	float fov = 45.0f;

	PerspectiveCameraNode();
	~PerspectiveCameraNode() = default;

	glm::mat4 get_view_matrix() const override;
	glm::mat4 get_projection_matrix() const override;
};
