/**
 * @file transform.h
 */

#pragma once

inline constexpr glm::vec3 VEC3_UP(0.0f, 1.0f, 0.0f);
inline constexpr glm::vec3 VEC3_RIGHT(1.0f, 0.0f, 0.0f);
inline constexpr glm::vec3 VEC3_FORWARD(0.0f, 0.0f, -1.0f);

inline constexpr glm::vec3 VEC3_ZERO(0.0f, 0.0f, 0.0f);
inline constexpr glm::vec3 VEC3_ONE(1.0f, 1.0f, 1.0f);

inline constexpr glm::vec3 WORLD_UP = VEC3_UP;

struct GL_API Transform {
	glm::vec3 position = VEC3_ZERO;
	glm::vec3 rotation = VEC3_ZERO;
	glm::vec3 scale = VEC3_ONE;

	void translate(const glm::vec3& p_translation);

	void rotate(float p_angle, glm::vec3 p_axis);

	glm::vec3 get_forward() const;
	glm::vec3 get_right() const;
	glm::vec3 get_up() const;

	glm::mat4 to_mat4() const;
};

inline constexpr Transform DEFAULT_TRANSFORM{};
