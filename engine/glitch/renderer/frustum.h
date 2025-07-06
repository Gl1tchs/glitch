/**
 * @file frustum.h
 *
 */

#pragma once

struct GL_API Frustum {
	glm::vec4 planes[6]; // left, right, bottom, top, near, far

	static Frustum from_view_proj(const glm::mat4 p_view_proj);
};

struct GL_API AABB {
	glm::vec3 min;
	glm::vec3 max;

	bool is_inside_frustum(const Frustum& p_frustum);

	AABB transform(const glm::mat4& p_transform);
};
