#include "glitch/core/transform.h"

#include <glm/gtc/matrix_transform.hpp>

void Transform::translate(const glm::vec3& p_translation) {
	position += p_translation;
}

void Transform::rotate(const float p_angle, const glm::vec3 p_axis) {
	rotation = rotation * glm::fquat(glm::radians(p_angle), p_axis);
}

glm::vec3 Transform::get_forward() const {
	return glm::normalize(rotation * VEC3_FORWARD);
}

glm::vec3 Transform::get_right() const {
	return glm::normalize(rotation * VEC3_RIGHT);
}

glm::vec3 Transform::get_up() const {
	return glm::normalize(rotation * VEC3_UP);
}

glm::mat4 Transform::to_mat4() const {
	glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
	transform *= glm::toMat4(rotation);
	transform = glm::scale(transform, scale);

	return transform;
}
