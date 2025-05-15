#pragma once

#include <glitch/renderer/camera.h>

class CameraController {
public:
	CameraController();

	void set_camera(Camera* p_camera, Transform* p_transform);

	void update(float p_dt);

	void set_speed(float p_value);
	float get_speed() const;

	void set_sensitivity(float p_value);
	float get_sensitivity() const;

private:
	Camera* camera;
	Transform* transform;

	const float SPEED_DEFAULT = 10.0f;
	const float SPEED_TURBO = 30.0f;

	float speed = SPEED_DEFAULT;

	float sensitivity = 0.05f;

	glm::vec2 last_mouse_pos;

	friend class Game;
};
