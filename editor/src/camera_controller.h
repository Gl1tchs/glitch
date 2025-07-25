#pragma once

#include <glitch/renderer/camera.h>

using namespace gl;

class CameraController {
public:
	CameraController();

	void set_camera(Camera* p_camera);

	void update(float p_dt);

	void set_speed(float p_value);
	float get_speed() const;

	void set_sensitivity(float p_value);
	float get_sensitivity() const;

private:
	Camera* camera;

	const float SPEED_DEFAULT = 1.0f;
	const float SPEED_TURBO = 3.0f;

	float speed = SPEED_DEFAULT;

	float sensitivity = 0.1f;
	float pitch = 0.0f;
	float yaw = 0.0f;

	glm::vec2 last_mouse_pos;

	friend class EditorApplication;
};
