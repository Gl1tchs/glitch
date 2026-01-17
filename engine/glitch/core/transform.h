/**
 * @file transform.h
 */

#pragma once

#include "glitch/core/core.h"

#include <glgpu/glgpu.h>

namespace gl {

inline constexpr Vec3f WORLD_UP = Vec3f::up();

struct GL_API Transform {
	const Transform* parent = nullptr;

	Vec3f local_position = Vec3f::zero();
	Vec3f local_rotation = Vec3f::zero();
	Vec3f local_scale = Vec3f::one();

	Vec3f get_position() const;
	Vec3f get_rotation() const;
	Vec3f get_scale() const;

	void translate(const Vec3f& translation);

	void rotate(float angle, Vec3f axis);

	Vec3f get_forward() const;
	Vec3f get_right() const;
	Vec3f get_up() const;

	Mat4 to_mat4() const;
};

inline constexpr Transform DEFAULT_TRANSFORM{};

GL_DEFINE_SERIALIZABLE(Transform, local_position, local_rotation, local_scale);

} //namespace gl
