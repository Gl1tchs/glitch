#include "glitch/renderer/frustum.h"

Frustum Frustum::from_view_proj(const glm::mat4 p_view_proj) {
	Frustum frustum;

	// Transpose because GLM matrices are column-major
	glm::mat4 m = glm::transpose(p_view_proj);

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
		float length = glm::length(glm::vec3(frustum.planes[i]));
		frustum.planes[i] /= length;
	}

	return frustum;
}

bool AABB::is_inside_frustum(const Frustum& p_frustum) {
	for (int i = 0; i < 6; ++i) {
		const glm::vec4& plane = p_frustum.planes[i];

		// Calculate the positive vertex (furthest point in direction of normal)
		glm::vec3 p = {
			plane.x >= 0 ? max.x : min.x,
			plane.y >= 0 ? max.y : min.y,
			plane.z >= 0 ? max.z : min.z,
		};

		// Plane equation: ax + by + cz + d > 0 is inside
		if (glm::dot(glm::vec3(plane), p) + plane.w < 0) {
			// fully outside
			return false;
		}
	}
	return true;
}

AABB AABB::transform(const glm::mat4& p_transform) {
	// Transform 8 corners and re-construct AABB
	glm::vec3 corners[8] = {
		{ min.x, min.y, min.z },
		{ max.x, min.y, min.z },
		{ min.x, max.y, min.z },
		{ min.x, min.y, max.z },
		{ max.x, max.y, min.z },
		{ max.x, min.y, max.z },
		{ min.x, max.y, max.z },
		{ max.x, max.y, max.z },
	};

	AABB result;
	result.min = glm::vec3(std::numeric_limits<float>::max());
	result.max = glm::vec3(std::numeric_limits<float>::lowest());

	for (int i = 0; i < 8; ++i) {
		glm::vec3 transformed =
				glm::vec3(p_transform * glm::vec4(corners[i], 1.0f));
		result.min = glm::min(result.min, transformed);
		result.max = glm::max(result.max, transformed);
	}

	return result;
}
