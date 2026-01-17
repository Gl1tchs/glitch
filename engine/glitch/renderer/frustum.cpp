#include "glitch/renderer/frustum.h"

#include <glgpu/math.h>

#include <limits>

namespace gl {

Frustum Frustum::from_view_proj(const Mat4& view_proj) {
	Frustum frustum;

	// Transpose the matrix (glgpu matrices are already column-major like GLM)
	Mat4 m = view_proj.transpose();

	// Left
	frustum.planes[0] = m[3] + m[0];
	// Right
	frustum.planes[1] = m[3] - m[0];
	// Bottom
	frustum.planes[2] = m[3] + m[1];
	// Top
	frustum.planes[3] = m[3] - m[1];
	// Near
	frustum.planes[4] = m[3] + m[2];
	// Far
	frustum.planes[5] = m[3] - m[2];

	// Normalize
	for (int i = 0; i < 6; ++i) {
		Vec3f plane_normal = Vec3f(frustum.planes[i].x, frustum.planes[i].y, frustum.planes[i].z);
		float length = plane_normal.length();
		frustum.planes[i] = frustum.planes[i] / length;
	}

	return frustum;
}

bool AABB::is_inside_frustum(const Frustum& frustum) const {
	for (int i = 0; i < 6; ++i) {
		const Vec4f& plane = frustum.planes[i];

		// Calculate the positive vertex (furthest point in direction of normal)
		Vec3f p = {
			plane.x >= 0 ? max.x : min.x,
			plane.y >= 0 ? max.y : min.y,
			plane.z >= 0 ? max.z : min.z,
		};

		// Plane equation: ax + by + cz + d > 0 is inside
		if (Vec3f(plane.x, plane.y, plane.z).dot(p) + plane.w < 0) {
			// fully outside
			return false;
		}
	}
	return true;
}

AABB AABB::transform(const Mat4& transform) const {
	// Transform 8 corners and re-construct AABB
	Vec3f corners[8] = {
		{ min.x, min.y, min.z },
		{ max.x, min.y, min.z },
		{ min.x, max.y, min.z },
		{ min.x, min.y, max.z },
		{ max.x, max.y, min.z },
		{ max.x, min.y, max.z },
		{ min.x, max.y, max.z },
		{ max.x, max.y, max.z },
	};

	AABB result = {
		.min = Vec3f(std::numeric_limits<float>::max()),
		.max = Vec3f(std::numeric_limits<float>::lowest()),
	};

	for (int i = 0; i < 8; ++i) {
		const Vec4f homogeneous_corner = Vec4f(corners[i], 1.0f);
		const Vec4f transformed_homogeneous = transform * homogeneous_corner;
		const Vec3f transformed = Vec3f(
				transformed_homogeneous.x, transformed_homogeneous.y, transformed_homogeneous.z);
		result.min = math::min(result.min, transformed);
		result.max = math::max(result.max, transformed);
	}

	return result;
}

} //namespace gl