#include "renderer/camera.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

glm::mat4 OrthographicCameraNode::get_view_matrix() const {
	return glm::inverse(transform.get_transform_matrix());
}

glm::mat4 OrthographicCameraNode::get_projection_matrix() const {
	return glm::ortho(-aspect_ratio * zoom_level, aspect_ratio * zoom_level,
			-zoom_level, zoom_level, near_clip, far_clip);
}

glm::mat4 PerspectiveCameraNode::get_view_matrix() const {
	return glm::lookAt(transform.get_position(),
			transform.get_position() + transform.get_forward(),
			transform.get_up());
}

glm::mat4 PerspectiveCameraNode::get_projection_matrix() const {
	glm::mat4 proj = glm::perspective(
			glm::radians(fov), aspect_ratio, near_clip, far_clip);

	// invert the Y direction on projection matrix so that we are more similar
	// to opengl and gltf axis
	proj[1][1] *= -1;

	return proj;
}
