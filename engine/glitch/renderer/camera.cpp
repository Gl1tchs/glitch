#include "glitch/renderer/camera.h"

#include <glgpu/math.h>

#include <cmath>

namespace gl {

OrthographicCamera::OrthographicCamera() : Camera() {
	near_clip = -1.0f;
	far_clip = 1.0f;
}

Mat4 OrthographicCamera::get_view_matrix(const Transform& transform) const {
	Mat4 transform_matrix = transform.to_mat4();
	return transform_matrix.inverse();
}

Mat4 OrthographicCamera::get_projection_matrix() const {
	return Mat4::ortho(-aspect_ratio * zoom_level, aspect_ratio * zoom_level, -zoom_level,
			zoom_level, near_clip, far_clip);
}

PerspectiveCamera::PerspectiveCamera() : Camera() {
	near_clip = 0.01f;
	far_clip = 10000.0f;
}

Mat4 PerspectiveCamera::get_view_matrix(const Transform& transform) const {
	return Mat4::look_at(transform.get_position(),
			transform.get_position() + transform.get_forward(), transform.get_up());
}

Mat4 PerspectiveCamera::get_projection_matrix() const {
	Mat4 proj = Mat4::perspective(math::as_radians(fov), aspect_ratio, near_clip, far_clip);

	// invert the Y direction on projection matrix so that we are more similar
	// to opengl and gltf axis
	proj[1][1] *= -1;

	return proj;
}

} //namespace gl