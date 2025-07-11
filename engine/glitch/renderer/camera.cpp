#include "glitch/renderer/camera.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

OrthographicCamera::OrthographicCamera() : Camera() {
	near_clip = -1.0f;
	far_clip = 1.0f;
}

glm::mat4 OrthographicCamera::get_view_matrix() const {
	return glm::inverse(transform.to_mat4());
}

glm::mat4 OrthographicCamera::get_projection_matrix() const {
	return glm::ortho(-aspect_ratio * zoom_level, aspect_ratio * zoom_level,
			-zoom_level, zoom_level, near_clip, far_clip);
}

PerspectiveCamera::PerspectiveCamera() : Camera() {
	near_clip = 0.01f;
	far_clip = 10000.0f;
}

glm::mat4 PerspectiveCamera::get_view_matrix() const {
	return glm::lookAt(transform.position,
			transform.position + transform.get_forward(), transform.get_up());
}

glm::mat4 PerspectiveCamera::get_projection_matrix() const {
	glm::mat4 proj = glm::perspective(
			glm::radians(fov), aspect_ratio, near_clip, far_clip);

	// invert the Y direction on projection matrix so that we are more similar
	// to opengl and gltf axis
	proj[1][1] *= -1;

	return proj;
}
