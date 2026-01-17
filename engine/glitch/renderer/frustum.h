/**
 * @file frustum.h
 *
 */

#pragma once

#include "glitch/core/defines.h"

#include <glgpu/glgpu.h>

namespace gl {

struct GL_API Frustum {
	Vec4f planes[6]; // left, right, bottom, top, near, far

	static Frustum from_view_proj(const Mat4& view_proj);
};

struct GL_API AABB {
	Vec3f min;
	Vec3f max;

	bool is_inside_frustum(const Frustum& frustum) const;

	AABB transform(const Mat4& transform) const;
};

} //namespace gl