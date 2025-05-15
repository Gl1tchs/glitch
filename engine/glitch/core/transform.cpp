#include "glitch/core/transform.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

glm::vec3 Transform::get_position() const {
	if (parent) {
		return local_position + parent->get_position();
	} else {
		return local_position;
	}
}

glm::vec3 Transform::get_rotation() const {
	if (parent) {
		return local_rotation + parent->get_rotation();
	} else {
		return local_rotation;
	}
}

glm::vec3 Transform::get_scale() const {
	if (parent) {
		return local_scale * parent->get_scale();
	} else {
		return local_scale;
	}
}

void Transform::translate(const glm::vec3& p_translation) {
	local_position += p_translation;
}

void Transform::rotate(const float p_angle, const glm::vec3 p_axis) {
	local_rotation += p_angle * p_axis;
}

glm::vec3 Transform::get_forward() const {
	glm::fquat orientation = glm::fquat(glm::radians(local_rotation));
	return glm::normalize(orientation * VEC3_FORWARD);
}

glm::vec3 Transform::get_right() const {
	glm::fquat orientation = glm::fquat(glm::radians(local_rotation));
	return glm::normalize(orientation * VEC3_RIGHT);
}

glm::vec3 Transform::get_up() const {
	glm::fquat orientation = glm::fquat(glm::radians(local_rotation));
	return glm::normalize(orientation * VEC3_UP);
}

glm::mat4 Transform::get_transform_matrix() const {
	glm::mat4 transform = glm::translate(glm::mat4(1.0f), local_position);

	transform =
			glm::rotate(transform, glm::radians(local_rotation.x), VEC3_RIGHT);
	transform = glm::rotate(transform, glm::radians(local_rotation.y), VEC3_UP);
	transform = glm::rotate(
			transform, glm::radians(local_rotation.z), VEC3_FORWARD);

	transform = glm::scale(transform, local_scale);

	if (parent) {
		transform = parent->get_transform_matrix() * transform;
	}

	return transform;
}
