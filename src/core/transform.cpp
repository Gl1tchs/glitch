#include "gl/core/transform.h"

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

void Transform::translate(glm::vec3 translation) {
	local_position += translation;
}

void Transform::rotate(const float angle, const glm::vec3 axis) {
	local_rotation += angle * axis;
}

void Transform::look_at(const glm::vec3& target) {
	glm::vec3 direction = glm::normalize(target - local_position);

	// Compute pitch and yaw angles using trigonometry
	float pitch = glm::degrees(asinf(-direction.y));
	float yaw = glm::degrees(atan2f(-direction.x, -direction.z));

	local_rotation = glm::vec3(pitch, yaw, 0.0f);
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

glm::vec3 Transform::get_direction() const {
	glm::vec3 direction(cos(local_rotation.x) * cos(local_rotation.y),
			sin(local_rotation.x),
			cos(local_rotation.x) * sin(local_rotation.y));
	direction = glm::normalize(direction);
	return direction;
}
