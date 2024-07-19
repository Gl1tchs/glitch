#pragma once

inline constexpr glm::vec3 VEC3_UP(0.0f, 1.0f, 0.0f);
inline constexpr glm::vec3 VEC3_RIGHT(1.0f, 0.0f, 0.0f);
inline constexpr glm::vec3 VEC3_FORWARD(0.0f, 0.0f, -1.0f);

inline constexpr glm::vec3 VEC3_ZERO(0.0f, 0.0f, 0.0f);
inline constexpr glm::vec3 VEC3_ONE(1.0f, 1.0f, 1.0f);

inline constexpr glm::vec3 WORLD_UP = VEC3_UP;

struct GL_API Transform {
	Transform* parent = nullptr;

	glm::vec3 local_position = VEC3_ZERO;
	glm::vec3 local_rotation = VEC3_ZERO;
	glm::vec3 local_scale = VEC3_ONE;

	glm::vec3 get_position() const;
	glm::vec3 get_rotation() const;
	glm::vec3 get_scale() const;

	void translate(glm::vec3 p_translation);

	void rotate(float p_angle, glm::vec3 p_axis);

	glm::vec3 get_forward() const;

	glm::vec3 get_right() const;

	glm::vec3 get_up() const;

	glm::mat4 get_transform_matrix() const;

	glm::vec3 get_direction() const;
};

inline constexpr Transform DEFAULT_TRANSFORM{};
