#pragma once

#include <gl/renderer/camera.h>

class CameraController {
public:
	CameraController();

	void set_camera(CameraNode* camera);

	void update(float dt);

	void set_speed(float value);
	float get_speed() const;

	void set_sensitivity(float value);
	float get_sensitivity() const;

private:
	CameraNode* camera;

	const float SPEED_DEFAULT = 10.0f;
	const float SPEED_TURBO = 30.0f;

	float speed = SPEED_DEFAULT;

	float sensitivity = 0.05f;

	glm::vec2 last_mouse_pos;
};
