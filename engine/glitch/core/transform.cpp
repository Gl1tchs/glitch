#include "glitch/core/transform.h"

#include <glgpu/math.h>

#include <memory>

namespace gl {

Vec3f Transform::get_position() const {
	if (parent) {
		return local_position + parent->get_position();
	} else {
		return local_position;
	}
}

Vec3f Transform::get_rotation() const {
	if (parent) {
		return local_rotation + parent->get_rotation();
	} else {
		return local_rotation;
	}
}

Vec3f Transform::get_scale() const {
	if (parent) {
		const Vec3f parent_scale = parent->get_scale();
		return Vec3f(local_scale.x * parent_scale.x, local_scale.y * parent_scale.y,
				local_scale.z * parent_scale.z);
	} else {
		return local_scale;
	}
}

void Transform::translate(const Vec3f& translation) { local_position += translation; }

void Transform::rotate(const float angle, const Vec3f axis) { local_rotation = axis * angle; }

Vec3f Transform::get_forward() const {
	const Mat4 rot_mat = Mat4::from_euler_angles(local_rotation);
	return Vec3f(rot_mat * Vec3f::forward()).normalize();
}

Vec3f Transform::get_right() const {
	const Mat4 rot_mat = Mat4::from_euler_angles(local_rotation);
	return Vec3f(rot_mat * Vec3f::right()).normalize();
}

Vec3f Transform::get_up() const {
	const Mat4 rot_mat = Mat4::from_euler_angles(local_rotation);
	return Vec3f(rot_mat * Vec3f::up()).normalize();
}

Mat4 Transform::to_mat4() const {
	Mat4 transform = Mat4::translate(local_position);
	transform = transform * Mat4::from_euler_angles(local_rotation);
	transform = transform * Mat4::scale(local_scale);

	if (parent) {
		transform = parent->to_mat4() * transform;
	}

	return transform;
}

} //namespace gl
